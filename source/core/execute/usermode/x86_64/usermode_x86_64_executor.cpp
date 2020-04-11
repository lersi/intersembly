#include "usermode_x86_64_executor.h" 

extern "C" void operation_end_handler(void);
extern "C" uint64_t start_execute(
	uint64_t register_struct_pointer, 
	uint64_t save_stack_pointer_to
);


using namespace execute;

X86_64_Executor::X86_64_Executor(void) :
m_environment_pages(INITIAL_PAGES_HOLDERS_COUNT),
m_inserted_code_offset(0),
m_changed_registers({0})
{
	(void)memset(
		&m_environment_registers,
		0,
		static_cast<int>(sizeof(m_environment_registers))
	);
	m_environment_code = {0};
	m_environment_stack = {0};
}

X86_64_Executor::~X86_64_Executor()
{
    try{
        IUsermodeExecutor::free_page(
                reinterpret_cast<address_t>(m_environment_code.array),
                m_environment_code.size
        );
        IUsermodeExecutor::free_page(
                reinterpret_cast<address_t>(m_environment_stack.array),
                m_environment_stack.size
        );

        for (array_t range : m_environment_pages)
        {
            IUsermodeExecutor::free_page(
                    reinterpret_cast<address_t>(range.array),
                    range.size
            );
        }
    }
    catch (std::exception) {
        /* do nothing, we did our best effort */
    }
}

void
X86_64_Executor::init(void)
{
	address_t code_base_address = IUsermodeExecutor::allocate_page(
			NULL_ADDRESS /* allocate random page */,
			page_size_e::PAGE_4KB
	);
	address_t stack_base_address = NULL_ADDRESS;
	m_environment_code.array = reinterpret_cast<uint8_t *>(code_base_address);
	m_environment_code.size = PAGE_4KB_SIZE;
	m_environment_registers.rip = code_base_address;

	stack_base_address = IUsermodeExecutor::allocate_page(
		code_base_address + PAGE_1GB_SIZE,
		page_size_e::PAGE_2MB
	);
	m_environment_stack.array = reinterpret_cast<uint8_t *>(
		stack_base_address
	);
	m_environment_stack.size = PAGE_2MB_SIZE;
	m_environment_registers.rsp = stack_base_address + m_environment_stack.size;

	/* saves current value of rflags to register_struct */
	asm volatile (R"(
		pushfq
		pop %0
	)"
	: "=r"(m_environment_registers.rflags)
	: /* no input */
	);
	m_environment_registers.rflags &= 0xffffffffffff7302;
}

void
X86_64_Executor::start_operation(void)
{
	address_t code_resume_address = reinterpret_cast<address_t>(&operation_end_handler);
	address_t * save_stack_address_to = reinterpret_cast<address_t *>(
		m_environment_code.array + (m_inserted_code_offset + __environment_stack_address_offset)
	);
	address_t * save_code_address_to = reinterpret_cast<address_t *>(
		m_environment_code.array + (m_inserted_code_offset + __environment_return_address_offset)
	);
	/* save the address of the code that resumes normal operation to the inserted code */
	*save_code_address_to = code_resume_address;

	asm volatile (
		".att_syntax\n"
		"movq %1, %%rax\n"
		"movq %2, %%rbx\n"
		"call start_execute\n"
		"movq %%rax, %0\n"
		: "=m"(m_changed_registers)
		: "r"(&m_environment_registers)/* the address of the register struct */,
			"r"(save_stack_address_to) /* the address of the place to save stack's value to */
		: "rax", "rbx"
	);
	m_environment_registers.rip -= __environment_remove_from_rip;
	/* the end */
	return;
}

void
X86_64_Executor::allocate_room_for(
	array_t & current_block_of_memory,
	const uint64_t desired_size
){
	address_t allocate_at = reinterpret_cast<address_t>(
		current_block_of_memory.array + current_block_of_memory.size
	);
	uint64_t size_to_allocate = desired_size - current_block_of_memory.size;
	address_t allocated_address = NULL_ADDRESS;
	uint64_t current_size_to_allocate = 0;
	page_size_e current_size_to_allocate_enum;

	/* align the allocation size */
	if (0 != size_to_allocate % PAGE_4KB_SIZE) {
		size_to_allocate += PAGE_4KB_SIZE - 
			(size_to_allocate % PAGE_4KB_SIZE);
	}
	

	while (size_to_allocate > 0)
	{
		if(0 == (allocate_at % PAGE_1GB_SIZE) && size_to_allocate >= PAGE_1GB_SIZE)
		{
			current_size_to_allocate = PAGE_1GB_SIZE;
			current_size_to_allocate_enum = page_size_e::PAGE_1GB;
		}
		else if (0 == (allocate_at % PAGE_2MB_SIZE) && size_to_allocate >= PAGE_2MB_SIZE)
		{
			current_size_to_allocate = PAGE_2MB_SIZE;
			current_size_to_allocate_enum = page_size_e::PAGE_2MB;
		}
		else
		{
			current_size_to_allocate = PAGE_4KB_SIZE;
			current_size_to_allocate_enum = page_size_e::PAGE_4KB;
		}
		
		
		allocated_address = IUsermodeExecutor::allocate_page(
			allocate_at, 
			current_size_to_allocate_enum
		);
		if (allocate_at == NULL_ADDRESS){
			allocate_at = allocated_address;
			current_block_of_memory.array = reinterpret_cast<uint8_t *>(allocated_address);
		}
		else if ( allocated_address != allocate_at)
		{
			IUsermodeExecutor::free_page(allocated_address, current_size_to_allocate);
			/** @todo throw proper exception or change the allocation function */
			throw std::exception();
		}

		current_block_of_memory.size += current_size_to_allocate;
		size_to_allocate -= current_size_to_allocate;
		allocate_at += current_size_to_allocate;
	}
	
}

bool
X86_64_Executor::prepare_execute(
	IN const array_t & opcodes_to_execute
) noexcept{
	bool result = false;
	uint64_t total_code_size = opcodes_to_execute.size + __environment_exit_instructions_size;

	if (0 == opcodes_to_execute.size) {
		/** @todo fill error */
		goto cleanup;
	}

	try
	{
		if (total_code_size > m_environment_code.size)
		{
			allocate_room_for(m_environment_code, total_code_size);
		}

		(void)memcpy(
			m_environment_code.array, 
			opcodes_to_execute.array, 
			opcodes_to_execute.size
		);
		m_inserted_code_offset = opcodes_to_execute.size;
		(void)memcpy(
			m_environment_code.array + opcodes_to_execute.size,
			__environment_exit_instructions,
			__environment_exit_instructions_size
		);

		m_environment_registers.rip = reinterpret_cast<address_t>(m_environment_code.array);
	}
	catch(std::exception e)
	{
		result = false;
		/** @todo fill error code */
		goto cleanup;
	}	

	result = true;
cleanup:
	return result;
}

bool
X86_64_Executor::execute(
	OUT common_registers_bit_mask_t & modified_registers
) noexcept{
	bool result = false;
	if (
        m_environment_registers.rip >= reinterpret_cast<address_t>(m_environment_code.array + m_environment_code.size)
        || m_environment_registers.rip < reinterpret_cast<address_t >(m_environment_code.array)
    ){
		/** @todo fill error */
		goto cleanup;
	}
	if (
        m_environment_registers.rsp > reinterpret_cast<address_t>(m_environment_stack.array + m_environment_stack.size)
        || m_environment_registers.rsp < reinterpret_cast<address_t >(m_environment_stack.array)
    ){
		/** @todo fill error */
		goto cleanup;
	}

	this->start_operation();
	modified_registers = m_changed_registers;

	result = true;
cleanup:
	return result;
}

bool
X86_64_Executor::get_common_registers(
	IN common_registers_bit_mask_t registers_to_read,
	OUT common_registers_t & registers
) const noexcept{
	bool result = false;
	uint64_t index = 0;

	/* check for array access out of range */
	if (registers_to_read.value >= 1 << (COMMON_REGISTERS_LENGTH + 1))
	{
		/** @todo fill error */
		goto cleanup;
	}

	while (registers_to_read.value != 0)
	{
		if (1 & registers_to_read.value)
		{
			registers.array[index] = m_environment_registers.array[index];
		}
		index++;
		registers_to_read.value >>= 1;
	}

	result = true;
cleanup:
	return result;
}
	

bool
X86_64_Executor::set_common_registers(
	IN common_registers_bit_mask_t registers_to_write,
	IN const common_registers_t & registers
) noexcept{
	bool result = false;
	uint64_t index = 0;

	/* check for array access out of range */
	if (registers_to_write.value >= 1 << (COMMON_REGISTERS_LENGTH + 1))
	{
		/** @todo fill error */
		goto cleanup;
	}

	while (registers_to_write.value != 0)
	{
		if (1 & registers_to_write.value)
		{
			m_environment_registers.array[index] = registers.array[index];
		}
		index++;
		registers_to_write.value >>= 1;
	}

	result = true;
cleanup:
	return result;
}

bool
X86_64_Executor::get_stack(
	OUT readonly_array_t & stack
) const noexcept{
	bool result = false;
	stack.str = m_environment_stack.array;
	stack.size = m_environment_stack.size;
	result = true;
	return result;
}


bool
X86_64_Executor::get_code_start_address(
	OUT address_t & start_address
) const noexcept{
	bool result = false;

	start_address = reinterpret_cast<address_t>(
		m_environment_code.array
	);

	result = true;
	return result;
}

bool
X86_64_Executor::load_static_module_to_memory(
	IN const array_t module_data,
	OUT address_t & load_address
) noexcept{
	bool result = false;
	array_t allocation_space = {.array= nullptr, .size= 0};

	try
	{
		X86_64_Executor::allocate_room_for(allocation_space, module_data.size);
		(void)memcpy(
			allocation_space.array,
			module_data.array,
			module_data.size
		);
		load_address = reinterpret_cast<address_t>(allocation_space.array);
		m_environment_pages.push_back(allocation_space);
	}
	catch(std::exception e){
		if (allocation_space.size != 0)
		{
			IUsermodeExecutor::free_page(
                    reinterpret_cast<address_t>(allocation_space.array),
                    allocation_space.size
            );
		}
		/** @todo fill error */
		goto cleanup;
	}

	result = true;
cleanup:
	return result;
}

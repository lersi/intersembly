#include "usermode_arm64_executor.h" 

extern "C" void restore_execution(void);
extern "C" uint64_t execute_asm(
	uint64_t register_struct_pointer, 
    uint64_t pointer_to_code,
	uint64_t stack_pointer_save_address
);


using namespace execute;

ARM64_Executor::ARM64_Executor(void) :
m_environment_pages(INITIAL_PAGES_HOLDERS_COUNT),
m_execution_code_size(0),
m_changed_registers({0}),
m_environment_registers_shadow_copy({0})
{
	(void)memset(
		&m_environment_registers,
		0,
		static_cast<int>(sizeof(m_environment_registers))
	);
	m_environment_code = {0};
	m_environment_stack = {0};
}

ARM64_Executor::~ARM64_Executor()
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
ARM64_Executor::init(void)
{
	address_t code_base_address = IUsermodeExecutor::allocate_page(
			NULL_ADDRESS /* allocate random page */,
			page_size_e::PAGE_4KB
	);
	address_t stack_base_address = NULL_ADDRESS;
	m_environment_code.array = reinterpret_cast<uint8_t *>(code_base_address);
	m_environment_code.size = PAGE_4KB_SIZE;
	m_environment_registers.PC = code_base_address;

	stack_base_address = IUsermodeExecutor::allocate_page(
		code_base_address + PAGE_1GB_SIZE,
		page_size_e::PAGE_2MB
	);
	m_environment_stack.array = reinterpret_cast<uint8_t *>(
		stack_base_address
	);
	m_environment_stack.size = PAGE_2MB_SIZE;
	m_environment_registers.SP = stack_base_address + m_environment_stack.size;
}

void
ARM64_Executor::start_operation(void)
{
	address_t code_resume_address = reinterpret_cast<address_t>(&restore_execution);
	address_t * save_stack_address_to = reinterpret_cast<address_t *>(
		m_environment_code.array + (__environment_pre_execute_instructions_size + 
                                    m_execution_code_size + 
                                    __environment_stack_address_offset)
	);
	address_t * save_code_address_to = reinterpret_cast<address_t *>(
		m_environment_code.array + (__environment_pre_execute_instructions_size + 
                                    m_execution_code_size + 
                                    __environment_return_address_offset)
	);
	/* save the address of the code that resumes normal operation to the inserted code */
	*save_code_address_to = code_resume_address;
	(void)memcpy(
		m_environment_registers_shadow_copy.array,
		m_environment_registers.array,
		COMMON_REGISTERS_LENGTH
	);

	/* the call to the execution assembly function is not standard, so use asm */
	asm volatile (
        "STP X0, X1, [SP, #-16]!\n"
        "STP X2, X30, [SP, #-16]!\n"
		"MOV X0, %0\n"
		"MOV X1, %1\n"
		"MOV X2, %2\n"
		"BL execute_asm\n"
		"LDP X2, X30, [SP], #16\n"
		"LDP X0, X1, [SP], #16\n"
		: 
		:   "r"(&m_environment_registers) /* the address of the register struct */,
            "r"(m_environment_code.array) /* the address of the code to execute */,   
			"r"(save_stack_address_to)    /* the address of the place to save stack's value to */
		: 
	);
	/* remove from PC the additinal code we added in order to restore execution */
	m_environment_registers.PC -= __environment_remove_from_pc;
	/* the end */
	return;
}

void
ARM64_Executor::allocate_room_for(
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
ARM64_Executor::prepare_execute(
	IN const array_t & opcodes_to_execute
) noexcept{
	bool result = false;
	uint64_t total_code_size = opcodes_to_execute.size + __environment_exit_instructions_size + __environment_pre_execute_instructions_size;

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

		/* copy the opcodes and execution restoration code into the code buffer */
		uint8_t * code_buffer = m_environment_code.array;
		(void)memcpy(
			code_buffer,
			__environment_pre_execute_instructions,
			__environment_pre_execute_instructions_size
		);
		code_buffer += __environment_pre_execute_instructions_size;
		(void)memcpy(
			code_buffer, 
			opcodes_to_execute.array, 
			opcodes_to_execute.size
		);
		m_execution_code_size = opcodes_to_execute.size;
		code_buffer += opcodes_to_execute.size;
		(void)memcpy(
			code_buffer,
			__environment_exit_instructions,
			__environment_exit_instructions_size
		);

		/* set program counter to the start of the code */
		m_environment_registers.PC = reinterpret_cast<address_t>(m_environment_code.array);
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
ARM64_Executor::execute(
	OUT common_registers_bit_mask_t & modified_registers
) noexcept{
	bool result = false;
	if (
        m_environment_registers.PC >= reinterpret_cast<address_t>(m_environment_code.array + m_environment_code.size)
        || m_environment_registers.PC < reinterpret_cast<address_t >(m_environment_code.array)
    ){
		/** @todo fill error */
		goto cleanup;
	}
	if (
        m_environment_registers.SP > reinterpret_cast<address_t>(m_environment_stack.array + m_environment_stack.size)
        || m_environment_registers.SP < reinterpret_cast<address_t >(m_environment_stack.array)
    ){
		/** @todo fill error */
		goto cleanup;
	}

	this->start_operation();
	m_changed_registers = this->calculate_modified_registers();
	modified_registers = m_changed_registers;

	result = true;
cleanup:
	return result;
}

bool
ARM64_Executor::get_common_registers(
	IN common_registers_bit_mask_t registers_to_read,
	OUT common_registers_t & registers
) const noexcept{
	bool result = false;
	uint64_t index = 0;

	/* check for array access out of range */
	if (registers_to_read.value >= (uint64_t)1 << (COMMON_REGISTERS_LENGTH + 1))
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
ARM64_Executor::set_common_registers(
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
ARM64_Executor::get_stack(
	OUT readonly_array_t & stack
) const noexcept{
	bool result = false;
	stack.str = m_environment_stack.array;
	stack.size = m_environment_stack.size;
	result = true;
	return result;
}


bool
ARM64_Executor::get_code_start_address(
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
ARM64_Executor::load_static_module_to_memory(
	IN const array_t module_data,
	OUT address_t & load_address
) noexcept{
	bool result = false;
	array_t allocation_space = {.array= nullptr, .size= 0};

	try
	{
		ARM64_Executor::allocate_room_for(allocation_space, module_data.size);
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

common_registers_bit_mask_t ARM64_Executor::calculate_modified_registers(void) 
{	
	common_registers_bit_mask_t result = {0};
	for (int i =0; i < COMMON_REGISTERS_LENGTH; i++){
		if(m_environment_registers.array[i] != m_environment_registers_shadow_copy.array[i]){
			result.value |= 1UL << i;
		}
	}
	return result;
}

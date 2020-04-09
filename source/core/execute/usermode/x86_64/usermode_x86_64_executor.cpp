#include "usermode_x86_64_executor.h" 

extern "C" void operation_end_handler(void);

asm (
".intel_syntax\n"
/* this code restores the executed code to our own code */
/* assumng the following:
	rax, rbx, rcx, rdx, rip are saved on the stack in that order
	rax - holds the pointer to stack to restore.
	rbx - holds the base address of this "function"

	in the restored stack the first value to pop is the pointer to the register struct,
	and the second value is the address to return to
*/
".global operation_end_handler\n"
/*".type operation_end_handler, @function\n"*/
"operation_end_handler:\n"
/* ==================================================================== */
/* switch stacks */
"	xchg rax, rsp\n"
/* load the pointer to register struct to rbx */
"	pop rbx\n"
/* ==================================================================== */
/* switch stacks */
"	xchg rax, rsp\n"
/* ==================================================================== */
/* save the value of rflags to the stack, we are going to use him */
"	pushf\n"
/* we need rax too, we will restore him later */
"   push rax\n"

/* rcx stores wich registers has changed */
" 	xor rcx, rcx\n"
/* save rsi and check if changed */
"	mov rdx, [rbx + 8 * 4]\n"
"	mov [rbx + 8 * 4], rsi\n"
"	cmp rsi, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save rdi and check if changed */
"	mov rdx, [rbx + 8 * 5]\n"
"	mov [rbx + 8 * 5], rdi\n"
"	cmp rdi, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save rbp and check if changed */
"	mov rdx, [rbx + 8 * 6]\n"
"	mov [rbx + 8 * 6], rbp\n"
"	cmp rbp, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* skip rsp bit position for later */
"	ror rcx\n"

/* save r8 and check if changed */
"	mov rdx, [rbx + 8 * 7]\n"
"	mov [rbx + 8 * 7], r8\n"
"	cmp r8, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r9 and check if changed */
"	mov rdx, [rbx + 8 * 9]\n"
"	mov [rbx + 8 * 9], r9\n"
"	cmp r9, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r10 and check if changed */
"	mov rdx, [rbx + 8 * 10]\n"
"	mov [rbx + 8 * 10], r10\n"
"	cmp r10, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r11 and check if changed */
"	mov rdx, [rbx + 8 * 11]\n"
"	mov [rbx + 8 * 11], r11\n"
"	cmp r11, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r12 and check if changed */
"	mov rdx, [rbx + 8 * 12]\n"
"	mov [rbx + 8 * 12], r12\n"
"	cmp r12, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r13 and check if changed */
"	mov rdx, [rbx + 8 * 13]\n"
"	mov [rbx + 8 * 13], r13\n"
"	cmp r13, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r14 and check if changed */
"	mov rdx, [rbx + 8 * 14]\n"
"	mov [rbx + 8 * 14], r14\n"
"	cmp r14, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* save r15 and check if changed */
"	mov rdx, [rbx + 8 * 15]\n"
"	mov [rbx + 8 * 15], r15\n"
"	cmp r15, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* now rsi contains the stack to return to */
"	pop rsi\n"
 
/* rax now stores the value of the Rflags register */
"	pop rax\n"
/* save Rflags and check if changed */
"	mov rdx, [rbx + 8 * 16]\n"
"	mov [rbx + 8 * 16], rax\n"
"	cmp rax, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* rax now stores the value of the rip register */
"	pop rax\n"
/* save rip and check if changed */
"	mov rdx, [rbx + 8 * 17]\n"
"	mov [rbx + 8 * 17], rax\n"
"	cmp rax, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	ror rcx\n"

/* if you want to add code that saves the status of the segment registers, add it here */

/* shift all the bits back, so the least sagnificant bit is the bit for rdx register */
"	shr rcx, 49\n"
/* store rcx in other register, we need rcx for loop count */
/* rdx now stores the bits of changed registers */
// "	mov rdx, rcx\n"
/* the loop is going to be executed 4 times, once for each register */
// "	mov rcx, 4\n"
/* the r8 register now stores the offset in the struct for the register */
"	mov r8, 24\n"
/* our register save loop */
"save_loop:\n"
/* rax now stores the original value of a register */
"	pop rax\n"
/* save the rgister and check if changed */
"	mov rdx, [rbx + r8]\n"
"	mov [rbx + r8], rax\n"
"	cmp rax, rdx\n"
"	setne al\n"
"	or cl, al\n"
"	rol rcx\n"
/* change the offset for the next register */
"	sub r8, 8\n"
"	jno save_loop\n"
/* when the loop ends, nthe stack pointer points into its otiginal position */
"	mov rdx, [rbx + 8 * 7]\n"
"	mov [rbx + 8 * 7], rsp\n"
"	cmp rdx, rsp\n"
"	je end\n"
/* set the specific bit of rsp, if has changes */
"	or rcx, 0b10000000\n"
"end:\n"
/* =========================================== */
/* stack switch */
"	mov rsp, rsi\n"
/* =========================================== */
/* mov to rax the bit mask of the changed registers */
"	mov rax, rcx\n"
/* return to normal code */
"	ret"
);

using namespace execute;

X86_64_Executor::X86_64_Executor(void) :
m_environment_pages(INITIAL_PAGES_HOLDERS_COUNT),
m_inserted_code_offset(0),
m_changed_registers({0})
{
	(void)memset(
		&m_environment_registers,
		static_cast<int>(sizeof(uint8_t)),
		static_cast<int>(sizeof(m_environment_registers) / sizeof (uint8_t))
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
	m_environment_code.array = reinterpret_cast<uint8_t *>(code_base_address);
	m_environment_code.size = PAGE_4KB_SIZE;

	m_environment_stack.array = reinterpret_cast<uint8_t *>(
		IUsermodeExecutor::allocate_page(
			code_base_address + PAGE_1GB_SIZE,
			page_size_e::PAGE_2MB
		)
	);
	m_environment_stack.size = PAGE_2MB_SIZE;
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
		".intel_syntax\n"
		/* save most registers to the stack */
		"push rax\n"
		"push rbx\n"
		"push rcx\n"
		"push rdx\n"
		"push rsi\n"
		"push rdi\n"
		"push rbp\n"
		"push r8\n"
		"push r9\n"
		"push r10\n"
		"push r11\n"
		"push r12\n"
		"push r13\n"
		"push r14\n"
		"push r15\n"
		"pushf\n"
		/* load to rax the address that we want to return to */
		"call after_execution\n" /* again a weard work around because compiler/assembler bugs */
		"pop rax\n"
		/* push that address onto stack */
		"push rax\n"
		/* load the environment register struct to rbx */
		// "mov rbx, %1\n"
		/* push that address onto stack */
		".att_syntax\n" /* the is a bug in the implementation of input / output macros, so it do not work with intel syntax */
		"push %1\n"


		/* load pointer to the address to store rsp */
		// "mov rax, %2\n"
		/* save rsp there */
		/* write the stack address to where it should be saved */
		"movq %%rsp, (%2)\n"
		".intel_syntax\n"

		/* the pointer to the struct is in rbx */		
		/* load most of the registers */
		"mov rcx, [rbx + 8 * 2]\n"
		"mov rdx, [rbx + 8 * 3]\n"
		"mov rsi, [rbx + 8 * 4]\n"
		"mov rdi, [rbx + 8 * 5]\n"
		"mov rbp, [rbx + 8 * 6]\n"
		"mov r8,  [rbx + 8 * 8]\n"
		"mov r9,  [rbx + 8 * 9]\n"
		"mov r10, [rbx + 8 * 10]\n"
		"mov r11, [rbx + 8 * 11]\n"
		"mov r12, [rbx + 8 * 12]\n"
		"mov r13, [rbx + 8 * 13]\n"
		"mov r14, [rbx + 8 * 14]\n"
		"mov r15, [rbx + 8 * 15]\n"

        /* =========================================================================================== */
		/* switch stacks */
		"mov rsp, [rbx + 8 * 7]\n"	
        /* =========================================================================================== */

		/* load the rflags register */
		"mov rax, [rbx + 8 * 16]\n"
		"push rax\n"
		"popf\n"

		/* load the value of rip to stack */
		"mov rax, [rbx + 8 * 17]\n"
		"push rax\n"

		/* load the value of rax */
		"mov rax, [rbx + 8 * 0]\n"
		/* load the value of rbx */
		"mov rbx, [rbx + 8 * 1]\n"
		/* jump to the code that we want to execute */
		"ret\n"
/* here assuming that the code has run, and all the registers of that code has been saved.
	also assuming that the first values to pop from the stack are the registers that we have saved */
"after_execution:\n"
	"	pop rax\n"
	"	call rax\n"

	"	popf\n"
	"	pop r15\n"
	"	pop r14\n"
	"	pop r13\n"
	"	pop r12\n"
	"	pop r11\n"
	"	pop r10\n"
	"	pop r9\n"
	"	pop r8\n"
	"	pop rbp\n"
	"	pop rdi\n"
	"	pop rsi\n"
	"	pop rdx\n"
	"	pop rcx\n"
	"	pop rbx\n"

	/* store changed registers bitmask at result */
	".att_syntax\n"
	"	movq %%rax, %0\n"
	".intel_syntax\n"

	"	pop rax\n"

		: "=b"(m_changed_registers)
		: "b"(&m_environment_registers)/* the address of the register struct */,
			"c"(save_stack_address_to) /* the address of the place to save stack's value to */
		: "rax"
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
        m_environment_registers.rsp >= reinterpret_cast<address_t>(m_environment_stack.array + m_environment_stack.size)
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

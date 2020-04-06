#include "usermode_x86_64_executor.h" 

extern "c" void operation_end_handler(void);

asm volatile (
".64\n"
".intel_syntax\n"
/* this code restores the executed code to our own code */
/* assumng the following:
	rax, rbx, rcx, rdx, rip are saved on the stack in that order
	rax - holds the pointer to stack to restore.
	rbx - holds the base address of this "function"

	in the restored stack the first value to pop is the pointer to the register struct,
	and the second value is the address to return to
*/
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
"	jno $save_loop\n"
/* when the loop ends, nthe stack pointer points into its otiginal position */
"	mov rdx, [rbx + 8 * 7]\n"
"	mov [rbx + 8 * 7], rsp\n"
"	cmp rdx, rsp\n"
"	je $end\n"
/* set the specific bit of rsp, if has changes */
"	or rcx, 0b1000 0000\n"
"end:\n"
/* =========================================== */
/* stack switch */
"	mov rsp, rsi\n"
/* =========================================== */
/* mov to rax the bit mask of the changed registers */
"	mov rax, rcx"
/* return to normal code */
"	ret"
);

using namespace execute;

X86_64_Executor::X86_64_Executor(void) :
m_environment_pages(INITIAL_PAGES_HOLDERS_COUNT)
{
	m_environment_registers = { 0 };
	m_environment_code = {0};
	m_environment_stack = {0};
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
		".64\n"
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
		"lea rax, $after_execution\n"
		/* push that address onto stack */
		"push rax\n"
		/* load the environment register struct to rbx */
		"mov rbx, %1\n"
		/* push that address onto stack */
		"push rbx\n"

		/* load pointer to the address to store rsp */
		"mov rax, %2\n"
		/* save rsp there */
		/* write the stack address to where it should be saved */
		"mov [rax], rsp\n"

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
	"	"
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

	"	mov %0, rax\n"

	"	pop rax\n"
	
		: "0"(m_changed_opcodes)
		: "1"(&m_environment_registers)/* the address of the register struct */,
			"2"(save_stack_address_to) /* the address of the place to save stack's value to */
		: "rax"
	);
	/* the end */
	return;
}



	


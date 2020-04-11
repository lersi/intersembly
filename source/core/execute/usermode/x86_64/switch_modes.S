.intel_syntax noprefix

/* this is the offset of register r8 in the register struct minus 8 */
.set r8_offset_in_struct, 56
/* this is the amount of registers that we are going to write on the struct for the first time */
.set amount_of_registers_after_r8, 10
/* this is the amount of registers that we are going to write on the struct for the second time */
.set amount_of_registers_second_loop, 3
/* this is the offset of register rsi in the register struct minus 8 */
.set rsi_offset_in_struct, 24
/* this is the amount of registers that we are going to write on the struct for the third time */
.set amount_of_registers_third_loop, 4
/* this is the offset of register rax in the register struct minus 8 */
.set rax_offset_in_struct, -8
/* this is the specific bit to change if rsp has changed */
.set rsp_bit, 0b10000000

.text
.global _operation_end_handler
.global operation_end_handler
_operation_end_handler:
operation_end_handler:
/* this code restores the executed code to our own code */
/* assumng the following:
	rax, rbx, rcx, rdx, rip are saved on the stack in that order
	rax - holds the pointer to stack to restore.
	rbx - holds the base address of this "function"

	in the restored stack the first value to pop is the pointer to the register struct,
	and the second value is the address to return to
*/
/* get rip value from stack */
    pop rdx
/* ==================================================================== */
/* switch stacks */
	xchg rax, rsp
/* now we are in the stack that we want to return to */
/* ==================================================================== */
/* load the pointer to register struct to rbx */
	pop rbx
/* save rax for now */
    push rax
/* push all registers to stack so it would be easier to handle them */
    push rsi
    push rdi
    push rbp
    /* do not push value of rsp, rsp is not yet in its original value */
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pushfq
    /* save rip on this stack */
    push rdx

/* rcx stores the amount of registers to store in the first loop */
    mov rcx, amount_of_registers_after_r8
/* zero rax, we are going to use him as the register bit field */
	xor rax, rax
/* save register r8 - r15, rflags and rip to the register struct */
first_save_loop:
/* rsi now stores the original value of a register */
    pop rsi
/* save the rgister and check if changed */
    mov rdx, [rbx + rcx * 8 + r8_offset_in_struct]
    mov [rbx + rcx * 8 + r8_offset_in_struct], rsi
    cmp rsi, rdx
    setne dl
    or al, dl
    rol rax
    loop first_save_loop

/* skip the bit of rsp */
	rol rax

/* rcx stores the amount of registers to store in the first loop */
	mov rcx, amount_of_registers_second_loop
/* save register rsi, rdi and rbp to the register struct */
second_save_loop:
/* rsi now stores the original value of a register */
    pop rsi
/* save the rgister and check if changed */
	mov rdx, [rbx + rcx * 8 + rsi_offset_in_struct]
	mov [rbx + rcx * 8 + rsi_offset_in_struct], rsi
	cmp rsi, rdx
    setne dl
    or al, dl
    rol rax
    loop second_save_loop

/* if you want to add code that saves the status of the segment registers, add it here */

/* restore rax's value (stack pointer) to rbp */
	pop rbp

/* ==================================================================== */
/* switch stacks */
	xchg rbp, rsp
/* now we are in the stack of the caller */
/* ==================================================================== */
 
	mov rcx, amount_of_registers_third_loop
/* our register save loop */
third_save_loop:
/* rax now stores the original value of a register */
	pop rsi
/* save the rgister and check if changed */
	mov rdx, [rbx + rcx * 8 + rax_offset_in_struct ]
	mov [rbx + rcx * 8 + rax_offset_in_struct], rsi
	cmp rsi, rdx
	setne dl
	or al, dl
	rol rax
	loop third_save_loop
	ror rax
	
/* when the loop ends, the stack pointer points into its original position */
	mov rdx, [rbx + 8 * 7]
	mov [rbx + 8 * 7], rsp
	cmp rdx, rsp
	je operation_end_handler_end
/* set the specific bit of rsp, if has changes */
	or rax, rsp_bit
operation_end_handler_end:
/* =========================================== */
/* stack switch */
	mov rsp, rbp
/* =========================================== */
/* return to normal code */
	ret

/* saves the current state and execute other code */
/* input parameters:
	rax - the register struct, to read registers from
	rbx - the place in the code to insert the address of rsp
	 */
/* output parameters:
	rax - will contain a bit field that determents which registers 
		has changed after the execution
	 */
.global start_execute
start_execute:
	/* save most registers to the stack */
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	pushfq
	/* push the address that we want to return to */
	call after_execution /* a work around for absolute addressing */
	/* push the pointer to the register struct */
	push rax

	/* rbx has the address to store rsp */
	/* save rsp there */
	/* write the stack address to where it should be saved */
	mov [rbx], rsp

	/* the pointer to the struct is in rbx */		
	/* load most of the registers */
	mov rcx, [rax + 8 * 2]
	mov rdx, [rax + 8 * 3]
	mov rsi, [rax + 8 * 4]
	mov rdi, [rax + 8 * 5]
	mov rbp, [rax + 8 * 6]
	mov r8,  [rax + 8 * 8]
	mov r9,  [rax + 8 * 9]
	mov r10, [rax + 8 * 10]
	mov r11, [rax + 8 * 11]
	mov r12, [rax + 8 * 12]
	mov r13, [rax + 8 * 13]
	mov r14, [rax + 8 * 14]
	mov r15, [rax + 8 * 15]

	/* =========================================================================================== */
	/* switch stacks */
	mov rsp, [rax + 8 * 7]	
	/* =========================================================================================== */

	/* load the rflags register */
	mov rbx, [rax + 8 * 16]
	push rbx
	popfq

	/* load the value of rip to stack */
	mov rbx, [rax + 8 * 17]
	push rbx

	/* load the value of rbx */
	mov rbx, [rax + 8 * 1]
	/* load the value of rax */
	mov rax, [rax + 8 * 0]
	/* jump to the code that we want to execute */
	ret

/* here assuming that the code has run, and all the registers of that code has been saved.
	also assuming that the first values to pop from the stack are the registers that we have saved */
after_execution:
	pop rdx
	call rdx
	popfq
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx

	ret


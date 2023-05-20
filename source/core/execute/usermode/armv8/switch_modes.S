.global execute_asm
// this function will execute the code and restore execution
// X0 - pointer to register struct
// X1 - pointer to the code to execute
// X2 - pointer to where to save sp
// x30 - the address to return to when finishing
execute_asm:
/* save all registers to the stack */
STP        X4, X5, [SP, #-16]!
STP        X6, X7, [SP, #-16]!
STP        X8, X9, [SP, #-16]!
STP        X10, X11, [SP, #-16]!
STP        X12, X13, [SP, #-16]!
STP        X14, X15, [SP, #-16]!
STP        X16, X17, [SP, #-16]!
STP        X18, X19, [SP, #-16]!
STP        X20, X21, [SP, #-16]!
STP        X22, X23, [SP, #-16]!
STP        X24, X25, [SP, #-16]!
STP        X26, X27, [SP, #-16]!
STP        X28, X29, [SP, #-16]!
MRS        X4, NZCV
STP        X30, X4, [SP, #-16]!
STP        X0, X3, [SP, #-16]! // save registers struct address and X3 onto the stack

/* load all registers values */
ADD        X3, X0, #40
LDP        X5, X6, [X3], #16
LDP        X7, X8, [X3], #16
LDP        X9, X10, [X3], #16
LDP        X11, X12, [X3], #16
LDP        X13, X14, [X3], #16
LDP        X15, X16, [X3], #16
LDP        X17, X18, [X3], #16
LDP        X19, X20, [X3], #16
LDP        X21, X22, [X3], #16
LDP        X23, X24, [X3], #16
LDP        X25, X26, [X3], #16
LDP        X27, X28, [X3], #16
LDP        X29, X30, [X3], #16
LDP        X3, X4, [X3] // X3 now containing the flags register, X4 contains the new stack
STP        X29, X30, [X4, #-16]! // save X30 and X29 into the other context stack (so they can be restored)
MOV        X30, X1 // set X30 to the address of code that we want to execute
MOV        X29, X3 // X29 now containing the flags register
/* SAVE STACK POINTER */
MOV        X3, SP
STR        X3, [X2]
/* switch stacks!!!! =============================== */
MOV        SP, X4 
LDP        X3, X4, [X0, #24]
LDP        X1, X2, [X0, #8]
LDR        X0, [X0]
MSR        NZCV, X29

RET // jumps to address in X30

.global restore_execution
// restores register state as before execution
// assumes the following
// original X0, X1 values are saved to the stack (of the other context)
// PC and flags registers also saved tp stack
// X0 - has the original stack address 
// X1 - the address of this function
restore_execution:
STP X2, X3, [SP, #-16]!
LDP X1, X3, [X0], #16 // X1 contains the register struct address, and X3 contains it's original value
/* save most registers into the struct */
ADD X2, X1, #32
STP X4, X5, [X2], #16
STP X6, X7, [X2], #16
STP X8, X9, [X2], #16
STP X10, X11, [X2], #16
STP X12, X13, [X2], #16
STP X14, X15, [X2], #16
STP X16, X17, [X2], #16
STP X18, X19, [X2], #16
STP X20, X21, [X2], #16
STP X22, X23, [X2], #16
STP X24, X25, [X2], #16
STP X26, X27, [X2], #16
STP X28, X29, [X2], #16
STR X30, [X2], #8
LDP X4, X5, [SP], #16 // X4 contains X2's execution context, X5 contains X3's execution context
LDP X6, X7, [SP], #16 // X6 contains PC value and X7 contains rflag
LDP X8, X9, [SP], #16 // X8 contains X0's execution context, X9 contains X1's execution context
MOV X10, SP // X10 now stores the execution state stack pointer
STP X7, X10, [X2], #16 // save flags and stack pointer to the struct
STR X6, [X2] // save th program counter onto the struct
STP X8, X9, [X1], #16 // save X0, X1 original values to the struct
STP X4, X5, [x1] // save X2, X3 original values to the struct

/* ================= switch stacks =============== */
MOV SP, X0 

/* restore the original value of registers before execution */
LDP X30, X0, [SP], #16 //X0 contains RFLAGS
LDP X28, X29, [SP], #16
LDP X26, X27, [SP], #16
LDP X24, X25, [SP], #16
LDP X22, X23, [SP], #16
LDP X20, X21, [SP], #16
LDP X18, X19, [SP], #16
LDP X16, X17, [SP], #16
LDP X14, X15, [SP], #16
LDP X12, X13, [SP], #16
LDP X10, X11, [SP], #16
LDP X8, X9, [SP], #16
LDP X6, X7, [SP], #16
LDP X4, X5, [SP], #16
MSR NZCV, X0
RET

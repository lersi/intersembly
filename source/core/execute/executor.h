#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "common.h"

#ifndef ADDRESS_T
typedef uint64_t address_t;
#endif

namespace execute{

    typedef union {
        uint64_t value;
        /* x86_64 registers */
        struct {
            bool rax : 1;
            bool rbx : 1;
            bool rcx : 1;
            bool rdx : 1;
            bool rsi : 1;
            bool rdi : 1;
            bool rbp : 1;
            bool rsp : 1;
            bool r8 : 1;
            bool r9 : 1;
            bool r10 : 1;
            bool r11 : 1;
            bool r12 : 1;
            bool r13 : 1;
            bool r14 : 1;
            bool r15 : 1;
            bool rflags : 1;
            bool rip : 1;
        };
    } common_registers_bit_mask_t;

    constexpr uint64_t COMMON_REGISTERS_LENGTH = 23;

    typedef union common_registers
    {
        uint64_t array[COMMON_REGISTERS_LENGTH];
        struct
        {
            /* General-Purpose Registers */
            uint64_t rax;
            uint64_t rbx;
            uint64_t rcx;
            uint64_t rdx;
            uint64_t rsi;
            uint64_t rdi;
            uint64_t rbp;
            uint64_t rsp;
            uint64_t r8;
            uint64_t r9;
            uint64_t r10;
            uint64_t r11;
            uint64_t r12;
            uint64_t r13;
            uint64_t r14;
            uint64_t r15;
            /* Program Status and Control Register */
            uint64_t rflags;

            /* Instruction Pointer */
            uint64_t rip;

            /* Segment Registers */
            uint64_t cs;
            uint64_t ds;
            uint64_t ss;
            uint64_t es;
            uint64_t fs;
            uint64_t gs;
        };
        struct
        {
            /* General-Purpose Registers */
            uint32_t eax; uint32_t rax_high;
            uint32_t ebx; uint32_t rbx_high;
            uint32_t ecx; uint32_t rcx_high;
            uint32_t edx; uint32_t rdx_high;
            uint32_t esi; uint32_t rsi_high;
            uint32_t edi; uint32_t rdi_high;
            uint32_t ebp; uint32_t rbp_high;
            uint32_t esp; uint32_t rsp_high;
            uint32_t e8;  uint32_t r8_high;
            uint32_t e9;  uint32_t r9_high;
            uint32_t e10; uint32_t r10_high;
            uint32_t e11; uint32_t r11_high;
            uint32_t e12; uint32_t r12_high;
            uint32_t e13; uint32_t r13_high;
            uint32_t e14; uint32_t r14_high;
            uint32_t e15; uint32_t r15_high;
            /* Program Status and Control Register */
            uint32_t eflags; uint32_t rflags_high;

            /* Instruction Pointer */
            uint32_t eip; uint32_t rip_high;

            /* Segment Registers */
/*            uint32_t cs; uint32_t cs_pad;
            uint32_t ds; uint32_t ds_pad;
            uint32_t ss; uint32_t ss_pad;
            uint32_t es; uint32_t es_pad;
            uint32_t fs; uint32_t fs_pad;
            uint32_t gs; uint32_t gs_pad;*/
        };
        
    } common_registers_t;
    
    

    class IExecutor
    {
    public:
        /**
         * @brief writes the code to execution enviroment
         * 
         * @note sets internaly stored instruction pointer to the start of this code
         * @note may override previous executions's code
         * 
         * @param[in] opcodes_to_execute array of machine code
         */
        virtual
        bool
        prepare_execute(
            IN const array_t & opcodes_to_execute
        ) noexcept = 0;

        /**
         * @brief executes the enviroment untill it fails or stop
         * 
         * @param[out] modified_registers will be set to zero, and every register that has been modified at this execution
         *              will turn on co-responding bit in this mask
         */
        virtual
        bool
        execute(
            OUT common_registers_bit_mask_t & modified_registers
        ) noexcept = 0;

        /**
         * @brief reads the common reginsters state in the enviroment
         * 
         * @param[in] registers_to_read bit mask that specifies with registers to read
         * @param[out] registers will be written with the necesery registers
         */
        virtual
        bool
        get_common_registers(
            IN const common_registers_bit_mask_t registers_to_read,
            OUT common_registers_t & registers
        ) const noexcept = 0;

        /**
         * @brief writes the common reginsters state in the enviroment
         * 
         * @param[in] registers_to_write bit mask that specifies with registers to write
         * @param[in] registers the struct to read registers value from
         */
        virtual
        bool
        set_common_registers(
                IN common_registers_bit_mask_t registers_to_write,
                IN const common_registers_t & registers
        ) noexcept = 0;

        /**
         * @brief get the entire stack of the enviroment
         * 
         * @param[out] stack an array that points directly to the enviroment's stack, should not be writtenable.
         */
        virtual
        bool
        get_stack(
            OUT readonly_array_t & stack
        ) const noexcept = 0;

        /**
         * @brief Get the start address of the code enviroment
         * 
         * @param[out] start_address will contain the start address of code executed by this executor
         */
        virtual
        bool
        get_code_start_address(
            OUT address_t & start_address
        ) const noexcept = 0;

        /**
         * @brief loads chunk of data to the enviroment of the executor.
         *          the access to that data would be read, write and execute.
         *          if the data is code, it must be pic and without any address translations.
         * 
         * @param[in] module_data contains the data to load to memory. 
         * @param[out] load_address will contain the base address of the loaded module.
         */
        virtual
        bool
        load_static_module_to_memory(
            IN const array_t module_data,
            OUT address_t & load_address
        ) noexcept = 0;
    };

}

#endif
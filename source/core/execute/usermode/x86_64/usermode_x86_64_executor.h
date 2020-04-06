#ifndef USERMODE_X86_64_EXECUTOR_H
#define USERMODE_X86_64_EXECUTOR_H

#include <core/execute/usermode/usermode_executor.h>

#include <list>
#include <exception>

namespace execute 
{
    class X86_64_Executor : public IUsermodeExecutor
    {
        private:
            static constexpr uint64_t __environment_remove_from_rip = 10;
            static constexpr uint64_t __environment_stack_address_offset = 12;
            static constexpr uint64_t __environment_return_address_offset = 22;
            static constexpr uint8_t __environment_exit_instructions[] = {
                0x50, /* push rax */
                0x53, /* push rbx */
                0x51, /* push rcx */
                0x52, /* push rdx */
                0xe8, 0x00, 0x00, 0x00, 0x00, /* call relative 0 (save rip onto stack) */
                0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov rax, stack_address
                (the address will be overriden, rax now stores the address of the stack t return to) */
                0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov rbx, address_to_jump_back 
                (the address will be overriden, rbx now stores the address to jump back to) */
                0xFF, 0xE3, /* call rbx (jumps back to code that will do the rest of the context switch) */
            };
            static constexpr uint64_t __environment_exit_instructions_size = sizeof(__environment_exit_instructions);
            static constexpr uint64_t INITIAL_PAGES_HOLDERS_COUNT = 10;

            /**
             * @brief struct that contains the state of the registers in the environment
             */
            common_registers_t m_environment_registers;
            /**
             * @brief will contain the start address of the code area and its size
             */
            array_t m_environment_code;
            /**
             * @brief will contain the start address of the stack and her size
             */
            array_t m_environment_stack;
            /**
             * @brief list of pages allocated while running the executor, so down the road we will free them
             */
            std::list<array_t> m_environment_pages;
            /**
             * @brief will contain the off set of our inserted code, relative to the start address of the enviroment code
             */
            uint64_t m_inserted_code_offset;
            common_registers_bit_mask_t m_changed_opcodes;
        protected:
        void
        start_operation(void);

        public:
        X86_64_Executor(void);
        ~X86_64_Executor();

        void
        init(void);

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
        ) noexcept override;

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
        ) noexcept override;

        /**
         * @brief reads the common reginsters state in the enviroment
         * 
         * @param[in] registers_to_read bit mask that specifies with registers to read
         * @param[out] registers will be written with the necesery registers
         */
        virtual
        bool
        get_common_registers(
            IN const common_registers_bit_mask_t & registers_to_read,
            OUT common_registers_t & registers
        ) noexcept override;

        /**
         * @brief writes the common reginsters state in the enviroment
         * 
         * @param[in] registers_to_write bit mask that specifies with registers to write
         * @param[in] registers the struct to read registers value from
         */
        virtual
        bool
        set_common_reginsters(
            IN const common_registers_bit_mask_t & registers_to_write,
            IN const common_registers_t & registers
        ) noexcept override;

        /**
         * @brief get the entire stack of the enviroment
         * 
         * @param[out] stack an array that points directly to the enviroment's stack, should not be writtenable.
         */
        virtual
        bool
        get_stack(
            OUT readonly_array_t & stack
        ) noexcept override;

        /**
         * @brief Get the start address of the code enviroment
         * 
         * @param[out] start_address will contain the start address of code executed by this executor
         */
        virtual
        bool
        get_code_start_address(
            OUT address_t & start_address
        ) noexcept override;

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
        ) noexcept override;

    };
}

#endif

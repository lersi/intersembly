#ifndef USERMODE_ARM64_EXECUTOR_H
#define USERMODE_ARM64_EXECUTOR_H

#include <core/execute/usermode/usermode_executor.h>

#include <list>
#include <exception>

namespace execute 
{
    class ARM64_Executor : public IUsermodeExecutor
    {
        private:
            static constexpr uint64_t __environment_remove_from_pc = 8;
            static constexpr uint64_t __environment_stack_address_offset = 28;
            static constexpr uint64_t __environment_return_address_offset = 36;
            static constexpr uint8_t __environment_pre_execute_instructions[] = {
                // 0xFE, 0x07, 0x41, 0xF8, /* LDR X30, [SP], #16;  */
                0xFD, 0x7B, 0xC1, 0xA8, /* LDP X29,X30, [SP], #16; load x29 and x30 from the stack and move the stack pointer */ 
            };
            static constexpr uint64_t __environment_pre_execute_instructions_size = sizeof(__environment_pre_execute_instructions);
            static constexpr uint8_t __environment_exit_instructions[] = {
                0xE0, 0x07, 0xBF, 0xA9, /* push X0,X1 into the stack */
                0x00, 0x00, 0x00, 0x10, /* copy PC value to X0 */
                0x01, 0x42, 0x3B, 0xD5, /* copy Rflags to X1 */
                0xE0, 0x07, 0xBF, 0xA9, /* push X0,X1 into the stack again, now as PC and flags */
                0x60, 0x00, 0x00, 0x58, /* load restore stack address to X0 */
                0x81, 0x00, 0x00, 0x58, /* load return address to X1 */
                0x20, 0x00, 0x1F, 0xD6, /* BR X1 (jumps back to code that will do the rest of the context switch) */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* stack_address (the address will be overriden) */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* address_to_jump_back (the address will be overriden) */
            };
            static constexpr uint64_t __environment_exit_instructions_size = sizeof(__environment_exit_instructions);
            static constexpr uint64_t INITIAL_PAGES_HOLDERS_COUNT = 10;

            /**
             * @brief struct that contains the state of the registers in the environment
             */
            common_registers_t m_environment_registers;
            /**
             * @brief contains a copy of the registers so modified registers could be detected
            */
            common_registers_t m_environment_registers_shadow_copy;
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
             * @brief will contain the offset of our inserted code, relative to the start address of the enviroment code
             */
            uint64_t m_execution_code_size;
            /**
             * @brief contains a bit field that indicates which registers has changed in the last execution
            */
            common_registers_bit_mask_t m_changed_registers;
        protected:
        /**
         * @brief executes the code
         */
        void
        start_operation(void);
        /**
         * @brief allocates more pages for an existing block of memory
         * 
         * @param[in,out] current_block_of_memory the block of memory to extend, the size value will be changed
         * @param[in] desired_size the new size for the block
         */
        static
        void
        allocate_room_for(
            array_t & current_block_of_memory,
            const uint64_t desired_size
        );

        /**
         * @brief checks which registers have changed and returns them in a bit mask
        */
        common_registers_bit_mask_t calculate_modified_registers(void);

        public:
        ARM64_Executor(void);
        ~ARM64_Executor();

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
            IN const common_registers_bit_mask_t registers_to_read,
            OUT common_registers_t & registers
        ) const noexcept override;

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
        ) const noexcept override;

        /**
         * @brief Get the start address of the code enviroment
         * 
         * @param[out] start_address will contain the start address of code executed by this executor
         */
        virtual
        bool
        get_code_start_address(
            OUT address_t & start_address
        ) const noexcept override;

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

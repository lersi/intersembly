#ifndef KEYSTONE_ASSEMBLER_H
#define KEYSTONE_ASSEMBLER_H

#include "common.h"
#include <core/assemble/assembler.h>

namespace assemble
{
    typedef enum {
        MODE_LITTLE_ENDIAN = 0,    // little-endian mode (default mode)
        MODE_BIG_ENDIAN = 1 << 30, // big-endian mode
        // arm / arm64
        MODE_ARM = 1 << 0,         // ARM mode
        MODE_THUMB = 1 << 4,       // THUMB mode (including Thumb-2)
        MODE_V8 = 1 << 6,          // ARMv8 A32 encodings for ARM
        // mips
        MODE_MICRO = 1 << 4,       // MicroMips mode
        MODE_MIPS3 = 1 << 5,       // Mips III ISA
        MODE_MIPS32R6 = 1 << 6,    // Mips32r6 ISA
        MODE_MIPS32 = 1 << 2,      // Mips32 ISA
        MODE_MIPS64 = 1 << 3,      // Mips64 ISA
        // x86 / x64
        MODE_16 = 1 << 1,          // 16-bit mode
        MODE_32 = 1 << 2,          // 32-bit mode
        MODE_64 = 1 << 3,          // 64-bit mode
        // ppc 
        MODE_PPC32 = 1 << 2,       // 32-bit mode
        MODE_PPC64 = 1 << 3,       // 64-bit mode
        MODE_QPX = 1 << 4,         // Quad Processing eXtensions mode
        // sparc
        MODE_SPARC32 = 1 << 2,     // 32-bit mode
        MODE_SPARC64 = 1 << 3,     // 64-bit mode
        MODE_V9 = 1 << 4,          // SparcV9 mode
    } keystone_mode;

    class KeystoneAssembler final : public IAssembler
    {
    private:
        void * m_keystone_engine;
    public:
        /**
         * @brief checks if the abi version of this program is compatible with keystone lib on your computer
         * 
         * @return true if the keystone lib's api is compatible
         * @return false if you keystone version is not yet supported or the version is no longer supported
         */
        static
        bool
        is_compatible_with_keystone_abi();

        KeystoneAssembler();
        ~KeystoneAssembler();

        /**
         * @brief inits the class, class initialization may fail
         * 
         * @param[in] architecture the cpu architecture to use
         * @param[in] architecture_mode a combination of ks_mode values to set more specificly architecture option 
         * @param[in] syntax  the assembly syntax to use
         */
        bool 
        init(
            IN const architecture_e architecture,
            IN const int architecture_mode,
            IN const assembly_syntax_e syntax
        );

        /**
         * @brief assembles an assembly string into opcode array.
         * 
         * @warning the returned pointer inside the array opcodes is
         *          dynamicly allocated and must be freed with the function
         *          free, that array can NOT be freed by any other function!
         * 
         * @param[in] assembly the assembly string to assemble
         * @param[out] opcodes an empty array struct that will be filled 
         *              with pointer to newly allocated opcode array.
         *              and the size of the opcode array.
         */
        virtual
        bool
        assemble(
            IN const string_t & assembly,
            OUT array_t & opcodes
        ) override;

        /**
         * @brief Set the syntax of the assembly to be parsed, can be changed in run time.
         * 
         * @param[in] syntax the assembly syntax to use.
         */
        virtual
        bool
        set_syntax(
            IN const assembly_syntax_e syntax
        ) override;

        /**
         * @brief frees arrays allocated by the the assemble function.
         * 
         * @param[in] opocode_array_to_release a pointer to array allocated by the function assemble.
         */
        virtual
        bool
        free(
            IN const uint8_t * opcode_array_to_release
        ) override;
    };

}
#endif
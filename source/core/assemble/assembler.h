#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "common.h"

namespace assemble
{
    typedef enum {
        ARM = 1,    // ARM architecture (including Thumb, Thumb-2)
        ARM64,      // ARM-64, also called AArch64
        MIPS,       // Mips architecture
        X86,        // X86 architecture (including x86 & x86-64)
        PPC,        // PowerPC architecture (currently unsupported)
        SPARC,      // Sparc architecture
        SYSTEMZ,    // SystemZ architecture (S390X)
        HEXAGON,    // Hexagon architecture
        EVM,        // Ethereum Virtual Machine architecture
    } architecture_e;
    
    typedef enum {
        INTEL = 1,
        ATNT
    } assembly_syntax_e;


    class IAssembler {
    public:
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
        ) = 0;

        /**
         * @brief Set the syntax of the assembly to be parsed, can be changed in run time.
         * 
         * @param[in] syntax the assembly syntax to use.
         */
        virtual
        bool
        set_syntax(
            IN const assembly_syntax_e syntax
        ) = 0;

        /**
         * @brief frees arrays allocated by the the assemble function.
         * 
         * @param[in] opocode_array_to_release a pointer to array allocated by the function assemble.
         */
        virtual
        bool
        free(
            IN const uint8_t * opocode_array_to_release
        ) = 0;
    };

}

#endif

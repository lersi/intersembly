#include <iostream>
#include <iomanip>

#include "common.h"

#include <core/assemble/assembler.h>
#include <core/assemble/keystone/keystone_assembler.h>



using namespace assemble;

char 
convert_byte_to_char(
    uint8_t byte,
    bool to_convert_high_part
){
    if (to_convert_high_part){
        byte >>= 4;
    } else {
        byte &= 0xF;
    }
    if (byte > 9){
        return 'A' + byte - 10;
    } else {
        return '0' + byte;
    }
}

int main(void){
    assembly_syntax_e syntax = assembly_syntax_e::INTEL;
    architecture_e arch = architecture_e::X86;
    KeystoneAssembler assembler;
    string_t assembly{str: nullptr, size: 0};
    array_t opcodes{array: nullptr, size: 0};
    std::string input;

    if(!assembler.init(
        arch, 
        keystone_mode::MODE_64 | keystone_mode::MODE_LITTLE_ENDIAN, 
        syntax)
    ){
        std::cerr << "assembler init error!";
        return 1;
    }

    std::ios cout_state(nullptr);
    while (true){
        std::cout << "enter assembly:" << std::endl;
        std::getline(std::cin, input);
        assembly.str = reinterpret_cast<const uint8_t *>(input.c_str());
        assembly.size = input.size() + 1;

        if (!assembler.assemble(assembly, opcodes))
        {
            std::cerr << "failed to assemble!" << std::endl;
        }
        else
        {
            std::cout << "result:" << std::endl;

            // cout_state.copyfmt(std::cout);
            // std::cout << std::hex << std::setfill('0') << std::setw(2);
            for (int i = 0; i < opcodes.size; i++){
                std::cout << convert_byte_to_char(opcodes.array[i], true) << convert_byte_to_char(opcodes.array[i], false);
                // std::cout << std::hex << std::setfill('0') << std::setw(2) << opcodes.array[i];
                if (15 == i % 16){
                    std::cout << '\n';
                }
                else if (3 == i % 4){
                    std::cout << ' ';
                }
            }

            std::cout << std::endl;
            // std::cout.copyfmt(cout_state);
        }
        
    }
}

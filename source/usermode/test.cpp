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

void
print_in_hex(
    const array_t & array,
    const bool reversed = false
){
    std::cout << "0x";
    if (reversed)
    {
        int index = 0;
        for (int i = array.size -1; i >= 0; i--, index++){
            std::cout << convert_byte_to_char(array.array[i], true) << convert_byte_to_char(array.array[i], false);
            if(16 == index % 16){
                std::cout << "\n0x";
            }
             else if (index % 2){
                std::cout << ' ';
            }
        }
    } 
    else 
    {
        for (int i = 0; i < array.size; i++){
            std::cout << convert_byte_to_char(array.array[i], true) << convert_byte_to_char(array.array[i], false);
            if (15 == i % 16){
                std::cout << "\n0x";
            }
            else if (i % 2){
                std::cout << ' ';
            }
        }
    }

    std::cout << std::endl;
}


int main(void){
    KeystoneAssembler assembler;
    string_t assembly{.str= nullptr, .size= 0};
    array_t opcodes{.array= nullptr, .size= 0};
    std::string input;

    if(!assembler.init(
        architecture_e::ARM64, 
        keystone_mode::MODE_LITTLE_ENDIAN, 
        assembly_syntax_e::NONE)
    ){
        std::cerr << "assembler init error!" << std::endl;
        return 1;
    }

    std::ios cout_state(nullptr);
    while (true){
        std::cout << "enter assembly:" << std::endl;
        std::getline(std::cin, input);
        

        if (0 == input.size()){
            break;
        }

        assembly.str = reinterpret_cast<const uint8_t *>(input.c_str());
        // assembly.str = reinterpret_cast<const uint8_t *>("push rsp");
        // assembly.size = strlen(reinterpret_cast<const char *>(assembly.str));
        assembly.size = input.size();

        if (!assembler.assemble(assembly, opcodes))
        {
            std::cerr << "failed to assemble!" << std::endl;
        }
        else
        {
            std::cout << "result:" << std::endl;
            print_in_hex(opcodes);

            
            if(!assembler.free(opcodes.array)){
                std::cerr << "failed to free opcodes!" << std::endl;
            }    
        }
        
    }
}

int use_sockets(void)
{
    return 0;
}

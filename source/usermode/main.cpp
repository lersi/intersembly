#include <iostream>
#include <iomanip>

#include "common.h"

#include <core/assemble/assembler.h>
#include <core/assemble/keystone/keystone_assembler.h>
#ifdef __X64__
#include <core/execute/usermode/x86_64/usermode_x86_64_executor.h>
#elif defined __ARM64__
#include <core/execute/usermode/armv8/usermode_arm64_executor.h>
#endif



using namespace assemble;
using namespace execute;

static const char * register_table_x86_64[] ={
        "rax   ",
        "rbx   ",
        "rcx   ",
        "rdx   ",
        "rsi   ",
        "rdi   ",
        "rbp   ",
        "rsp   ",
        "r8    ",
        "r9    ",
        "r10   ",
        "r11   ",
        "r12   ",
        "r13   ",
        "r14   ",
        "r15   ",
        "rflags",
        "rip   ",
        "cs    ",
        "ds    ",
        "ss    ",
        "es    ",
        "fs    ",
        "gs    ",
};
static const char * register_table_arm64[] = {
    "X0   ",
    "X1   ",
    "X2   ",
    "X3   ",
    "X4   ",
    "X5   ",
    "X6   ",
    "X7   ",
    "X8   ",
    "X9   ",
    "X10  ",
    "X11  ",
    "X12  ",
    "X13  ",
    "X14  ",
    "X15  ",
    "X16  ",
    "X17  ",
    "X18  ",
    "X19  ",
    "X20  ",
    "X21  ",
    "X22  ",
    "X23  ",
    "X24  ",
    "X25  ",
    "X26  ",
    "X27  ",
    "X28  ",
    "X29  ",
    "LR   ",
    "flags",
    "SP   ",
    "PC   ",
};

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

void
print_registers(
    const IExecutor & executor,
    common_registers_bit_mask_t registers_to_print
){
#ifdef __X64__
    const char ** register_table = register_table_x86_64;
#elif defined __ARM64__
    const char ** register_table = register_table_arm64;
#endif
    common_registers_t read_registers = { .array = {0} };
    array_t register_as_array = {0};
    uint64_t register_index = 0;

    if(!executor.get_common_registers(registers_to_print, read_registers)){
        std::cerr << "failed to read registers" << std::endl;
        return;
    }

    /* no bound check here, assuming that get_common_registers will fail if there is a bit aot side of range */
    while (registers_to_print.value != 0)
    {
        if (1 & registers_to_print.value)
        {
            std::cout << register_table[register_index] << " ";

            register_as_array.array = reinterpret_cast<uint8_t *>(read_registers.array + register_index);
            register_as_array.size = sizeof(read_registers.array[register_index]);

            print_in_hex(register_as_array, /* reversed= */ true);
        }
        registers_to_print.value >>= 1;
        register_index++;
    }
}

int main(void){
    KeystoneAssembler assembler;
    string_t assembly{.str= nullptr, .size= 0};
    array_t opcodes{.array= nullptr, .size= 0};
    common_registers_bit_mask_t changed_registers = {0};
    std::string input;
#ifdef __X64__
    X86_64_Executor executor;
    if(!assembler.init(
        architecture_e::X86, 
        keystone_mode::MODE_64, 
        assembly_syntax_e::INTEL)
    ){
        std::cerr << "assembler init error!";
        return 1;
    }
#elif defined __ARM64__
    ARM64_Executor executor;
    if(!assembler.init(
        architecture_e::ARM64, 
        keystone_mode::MODE_LITTLE_ENDIAN, 
        assembly_syntax_e::NONE)
    ){
        std::cerr << "assembler init error!";
        return 1;
    }
#endif
        
    try{
        executor.init();
    }
    catch (std::exception e)
    {
        std::cerr << "executor init error!";
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
        assembly.size = input.size();

        if (!assembler.assemble(assembly, opcodes))
        {
            std::cerr << "failed to assemble!" << std::endl;
        }
        else
        {
            std::cout << "result:" << std::endl;
            print_in_hex(opcodes);

            std::cout << "starting to execute..." << std::endl;
            if(!executor.prepare_execute(opcodes)){
                std::cerr << "failed to prepare execution!" << std::endl;
            }
            else
            {
                if(!assembler.free(opcodes.array)){
                    std::cerr << "failed to free opcodes!" << std::endl;
                }
                if(!executor.execute(changed_registers)){
                    std::cerr << "failed to execute!" << std::endl;
                }
            }

            print_registers(executor, changed_registers);
        }
        
    }
}

int use_sockets(void)
{
    return 0;
}

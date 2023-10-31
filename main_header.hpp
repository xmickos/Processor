#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stack_headers.hpp"

#define DEBUG

#define CHAR_LOGIC


#pragma once

#define CPU_PC_SIZE 128

#define MAX_POINTERNAME_LEN 128

#define STDOUT_LINE_LEN 150

struct Processor{
    Stack stk;
    Elem_t RAX = 0, RBX = 0, RCX = 0, RDX = 0;
    size_t programm_counter = 0;
    unsigned char cs[CPU_PC_SIZE] = {};
};

struct MyFileStruct{
    char* buff = nullptr;
    size_t num_of_str = 0;
    size_t filesize = 0;
};

struct MyPointer{
    char name[MAX_POINTERNAME_LEN] = {};
    ssize_t address = -1;
};

#define INIT_LEN 25

#define VM_POISON -2

#define SINGLE_BIT(bit) (1U << (unsigned) bit)

#define STRINGIFY(val) #val

#ifdef DEBUG
#define CHECKPOINT(message) do{ printf("Checkpoint " message "\n"); fprintf(logfile, "Checkpoint " message "\n"); } while(0)
#else
#define CHECKPOINT(message) ;
#endif

#define CPU_VERIF(condition, message)     do{ if(condition){                                                          \
        fprintf(logfile, "[CPU Verificator] " message "\n");                                                          \
        return -1;                                                                                                    \
    } }while(0)                                                                                                       \

#define COMMAND_COMPARE_ASM(arg, value, curr_command, int_command)         if(!strcmp(curr_command, arg)){            \
            int_command = value;                                                                                      \
        }                                                                                                             \

#define BITWISE_COMPARE_ASM(curr_command, arg, value, bit_command)         if(!strcmp(curr_command, arg)){            \
            bit_command |= value;                                                                                     \
        }                                                                                                             \

#define REGISTER_COMPARE_ASM(arg, value, curr_arg, int_arg)        if(!strcmp(curr_arg, arg)){                        \
            int_arg = value;                                                                                          \
        }                                                                                                             \



#define COMMAND_COMPARE_DISASM(string_arg, value, silent_arg, int_command, out_str)         if(int_command == value){ \
            printf("%s!  Silent: %d\n", string_arg, silent_arg);                                                      \
            strcpy(out_str, string_arg);                                                                              \
        }                                                                                                             \

#define SKIP_STR() do{ while(*buff != '\n' && *buff != '\0') buff++;                                                  \
                        buff++;  } while(0)                                                                           \

#define REG_ASSIGN(cpu, logfile, register_code, value)                     case register_code:                        \
                        cpu->register_code = value;                                                                   \
                        break;                                                                                        \

#define PUSH_FR_REG(stk, logfile, register_code)                     case register_code:                              \
                        StackPush(stk, logfile, cpu->register_code);                                                  \
                        break;                                                                                        \

#define COPY_FR_REG(register, register_name, out_str)                     case register_name:                         \
                        strcpy(out_str, #register_name);                                                              \
                        break;                                                                                        \

#define WRITE_INT(pos_index, argument)     *(int *)(char_binary_code + pos_index) = argument                          \

#ifdef DEBUG
#define SWITCH_ECHO(COMMAND_NAME)  do{ printf("Case" #COMMAND_NAME "\n");                                             \
                fprintf(logfile, "Case" #COMMAND_NAME "\n");} while(0)
#else
#define SWITCH_ECHO(COMMAND_NAME) ;
#endif

#define POINTERS_DUMP(pointers, pointers_counter) do{         printf("pointers at %d line:\n", __LINE__);             \
        for(size_t i = 0; i < pointers_counter; i++){                                                                 \
            printf("(%s, %3zd)\n", pointers[i].name, pointers[i].address);                                            \
        } }while(0)

#define BEAUTY_BIN_DUMP(message) do{        printf(message ": command: %s, i = %d\n", curr_command, i);               \
        for(size_t j = 0; j <= binary_pos_counter; j++){                                                              \
            printf("%4d", char_binary_code[j]);                                                                      \
        }                                                                                                             \
        printf("\n");                                                                                                 \
        for(int k = 0; k < i; k++){                                                                                   \
            if(k - i == 1) printf("   ");                                                                             \
            else printf("    ");                                                                                      \
        }                                                                                                             \
        printf("   \033[1;34m^\033[0m\n"); }while(0)

#define STRIKE_ME_OUT() do{ for(size_t trash = 0; trash < STDOUT_LINE_LEN; trash++) printf("–"); }while(0)            \


enum FUNC_CODES{
    CPU_VERSION     = (9),
    NUM_OF_REGS     = (4),
    NUM_OF_COMMANDS = (11),
    CPU_CS_SIZE     = (128),
    CPU_INIT_CAP    = (10),
    REGISTER_BIT    = SINGLE_BIT(7),
    RPOP            = SINGLE_BIT(3),
    IMREG_BIT       = (0b10000000u),
    REGISTER_BITS   = (0b01100000u),
    COMMAND_BITS    = (0b00011111u)
};

#define MASK_LOWER(bits) ((1U << (bits)) - 1)

enum BIT_FUNC_CODES{
    PUSH   = (0b00000001),
    POP    = (0b00000100),
    DIV    = (0b00000010),
    SUB    = (0b00000011),
    OUT    = (0b00000101),
    IN     = (0b00000110),
    MUL    = (0b00000111),
    ADD    = (0b00001001),
    SQRT   = (0b00001000),
    HLT    = (0b11111111),
    JMPFLG = (0b11111110),
    RAX    = (0b00000000),
    RBX    = (0b00100000),
    RCX    = (0b01000000),
    RDX    = (0b01100000),
    JMP    = (0b00001010),
    JB     = (0b00001011)
};

int read_from_file(const char* filename, MyFileStruct *FileStruct, FILE* logfile);

int string_processing_asm(MyFileStruct* FileStruct, FILE* output, FILE* bin_output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

int kernel(Processor *cpu, FILE* logfile, const unsigned char* bin_buff);

int CpuDtor(Processor *cpu, FILE* logfile);

uint32_t CpuVerificator(Processor *cpu, FILE* logfile);

uint32_t CpuDump(Processor *cpu, FILE* logfile);

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile);

unsigned char* read_from_bin_file(const char* filename, FILE* logfile);

int simple_pointer_search(MyPointer *pointers, char *name, int pointers_counter);

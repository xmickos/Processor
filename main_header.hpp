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

#define INIT_LEN 25

#define VM_POISON -2

#define SINGLE_BIT(bit) (1U << (unsigned) bit)

#define STRINGIFY(val) #val

#ifdef DEBUG
#define CHECKPOINT(message) do{ printf("Checkpoint " message "\n"); fprintf(logfile, "Checkpoint " message "\n"); } while(0)
#else
#define CHECKPOINT(message) ;
#endif

#define CPU_VERIF(condition, message)     do{ if(condition){                               \
        fprintf(logfile, "[CPU Verificator] " message "\n");                               \
        return -1;                                                                         \
    } }while(0)                                                                            \

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

#define SKIP_STR() do{ while(*buff != '\n' && *buff != '\0') buff++;                                 \
                        buff++;  } while(0)                                                          \

#define REG_ASSIGN(cpu, logfile, register_name, register_code, value)                     case register_code:                      \
                        cpu->register_name = value;                                                                                \
                        break;                                                                                                     \

#define PUSH_FR_REG(stk, logfile, register_name, register_code)                     case register_code:     \
                        StackPush(stk, logfile, cpu->register_name);                                        \
                        break;                                                                              \

#define COPY_FR_REG(register, register_name, out_str)                     case register_name:             \
                        strcpy(out_str, #register_name);                                             \
                        break;                                                                       \

#ifdef DEBUG
#define SWITCH_ECHO(COMMAND_NAME)  do{ printf("Case" #COMMAND_NAME "\n");                            \
                fprintf(logfile, "Case" #COMMAND_NAME "\n");} while(0)
#else
#define SWITCH_ECHO(COMMAND_NAME) ;
#endif


enum FUNC_CODES{
    PUSH  = (1),
    RPUSH = (33),
    DIV   = (2),
    SUB   = (3),
    POP   = (43),
    OUT   = (5),
    IN    = (6),
    MUL   = (7),
    ADD   = (9),
    SQRT  = (8),
    HLT   = (-1),
    RAX   = (1),
    RBX   = (2),
    RCX   = (3),
    RDX   = (4),
    CPU_VERSION     = (9),
    NUM_OF_REGS     = (4),
    NUM_OF_COMMANDS = (11),
    CPU_CS_SIZE     = (128),
    CPU_INIT_CAP    = (10),
    REGISTER_BIT    = SINGLE_BIT(7),
    RPOP            = SINGLE_BIT(3),
    IMREG_BIT       = (0b10000000u),
    REGISTER_BITS   = (0b01100000u),
    COMMAND_BITS    = (0b00011111u),
    FIRST_INT_BYTE  = (0b11111111 << 24),
    SECOND_INT_BYTE = (0b0000000011111111 << 16),
    THIRD_INT_BYTE  = (0b000000000000000011111111 << 8)
};

enum BIT_FUNC_CODES{
    BPUSH = (0b00000001),
    BPOP  = (0b00000100),
    BDIV  = (0b00000010),
    BSUB  = (0b00000011),
    BOUT  = (0b00000101),
    BIN   = (0b00000110),
    BMUL  = (0b00000111),
    BADD  = (0b00001001),
    BSQRT = (0b00001000),
    BHLT  = (0b11111111),
    BRAX  = (0b00000000),
    BRBX  = (0b00100000),
    BRCX  = (0b01000000),
    BRDX  = (0b01100000)
};

int read_from_file(const char* filename, MyFileStruct *FileStruct, FILE* logfile);

int string_processing_asm(MyFileStruct* FileStruct, FILE* output, FILE* bin_output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

int kernel(const char* buff, Processor *cpu, FILE* logfile, const unsigned char* bin_buff);

int CpuDtor(Processor *cpu, FILE* logfile);

uint32_t CpuVerificator(Processor *cpu, FILE* logfile);

uint32_t CpuDump(Processor *cpu, FILE* logfile);

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile);

unsigned char* read_from_bin_file(const char* filename, FILE* logfile);

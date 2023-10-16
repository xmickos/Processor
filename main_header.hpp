#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stack_headers.hpp"

#define DEBUG


#pragma once

#define CPU_PC_SIZE 128

struct Processor{
    Stack stk;
    Elem_t RAX = 0, RBX = 0, RCX = 0, RDX = 0;
    size_t programm_counter = 0;
    int cs[CPU_PC_SIZE] = {};
};

#define INIT_LEN 25

#define VM_POISON -2

#define STRINGIFY(val) #val

#define CPU_VERIF(condition, message)     do{ if(condition){                               \
        fprintf(logfile, "[CPU Verificator] " message "\n");                               \
        return -1;                                                                         \
    } }while(0)                                                                            \

#define COMMAND_COMPARE_ASM(arg, value, curr_command, int_command)         if(!strcmp(curr_command, arg)){            \
            int_command = value;                                                                                      \
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

#define REG_ASSIGN(cpu, logfile, register, value)                     case register:                 \
                        cpu->register = value;                                                       \
                        break;                                                                       \

#define PUSH_FR_REG(stk, logfile, register)                     case register:                       \
                        StackPush(stk, logfile, cpu->register);                                      \
                        break;                                                                       \

#define COPY_FR_REG(register, register_name, out_str)                     case register:             \
                        strcpy(out_str, #register_name);                                             \
                        break;                                                                       \

#ifdef DEBUG
#define SWITCH_ECHO(COMMAND_NAME)  do{ printf("Case" #COMMAND_NAME "\n");                            \
                fprintf(logfile, "Case" #COMMAND_NAME "\n");} while(0)
#else
#define SWITCH_ECHO(COMMAND_NAME) ;
#endif


enum FUNC_CODES{
    PUSH = (1),
    RPUSH= (33),
    DIV  = (2),
    SUB  = (3),
    POP  = (43),
    OUT  = (5),
    IN   = (6),
    MUL  = (7),
    ADD  = (9),
    SQRT = (8),
    HLT  = (-1),
    RAX  = (1),
    RBX  = (2),
    RCX  = (3),
    RDX  = (4),
    CPU_VERSION = (9),
    NUM_OF_REGS = (4),
    NUM_OF_COMMANDS = (11),
    CPU_CS_SIZE = (128)
};

char* read_from_file(char* filename, FILE* logfile);

void string_processing_asm(char* buff, FILE* output, FILE* bin_output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

int kernel(const char* buff, Processor *cpu, FILE* logfile, const int* bin_buff);

int CpuDtor(Processor *cpu, FILE* logfile);

uint32_t CpuVerificator(Processor *cpu, FILE* logfile);

uint32_t CpuDump(Processor *cpu, FILE* logfile);

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile);

int* read_from_bin_file(char* filename, FILE* logfile);

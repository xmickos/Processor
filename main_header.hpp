#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stack_headers.hpp"

// #define DEBUG

#pragma once

struct Processor{
    Stack stk;
    Elem_t rax = 0, rbx = 0, rcx = 0, rdx = 0;
};

#define CPU_VERIF(condition, message)     do{ if(condition){                               \
        fprintf(logfile, "[CPU Verificator] " message "\n");                               \
        return -1;                                                                         \
    } }while(0)                                                                            \

#define COMMAND_COMPARE_ASM(arg, value)         if(!strcmp(curr_command, arg)){            \
            int_command = value;                                                           \
        }                                                                                  \

#define REGISTER_COMPARE_ASM(arg, value)        if(!strcmp(curr_arg, arg)){                \
            int_arg = value;                                                               \
        }                                                                                  \


#define COMMAND_COMPARE_DISASM(string_arg, value)         if(atoi(curr_command) == value){           \
            printf("%s!  Silent: %d\n", string_arg, silent_arg);                                     \
            strcpy(curr_command, string_arg);                                                        \
        }                                                                                            \

#define SKIP_STR() do{ while(*buff != '\n' && *buff != '\0') buff++;                                 \
                        buff++;  } while(0)                                                          \


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
    RDX  = (4)
};

char* read_from_file(char* filename, FILE* logfile);

void string_processing_asm(char* buff, FILE* output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

int kernel(const char* buff, Processor *cpu, FILE* logfile);

int CpuDtor(Processor *cpu, FILE* logfile);

uint32_t CpuVerificator(Processor *cpu, FILE* logfile);

uint32_t CpuDump(Processor *cpu, FILE* logfile);

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile);

#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#include <string.h>
#include "stack_headers.hpp"


#define COMMAND_COMPARE_ASM(arg, value)         if(!strcmp(curr_command, arg)){            \
            int_command = value;                                                           \
        }                                                                                  \

#define COMMAND_COMPARE_DISASM(string_arg, value)         if(atoi(curr_command) == value){           \
            printf("%s!  Silent: %d\n", string_arg, silent_arg);                                     \
            strcpy(curr_command, string_arg);                                                        \
        }                                                                                            \

#define SKIP_STR() do{ while(*buff != '\n' && *buff != '\0') buff++;                                 \
                        buff++;  } while(0)                                                          \


enum FUNC_CODES{
    PUSH = (1),
    DIV  = (2),
    SUB  = (3),
    POP  = (4),
    OUT  = (5),
    IN   = (6),
    MUL  = (7),
    ADD  = (9),
    SQRT = (8),
    HLT  = (-1)
};

char* read_from_file(char* filename, FILE* logfile);

void string_processing_asm(char* buff, FILE* output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

void general_cpu_func(const char* buff, Stack *stk, FILE* logfile);

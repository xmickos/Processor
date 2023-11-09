#include <stdio.h>
#include <stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "stack_headers.hpp"

#define DEBUG

#define CHAR_LOGIC


#pragma once

#define CPU_CS_CAPACITY 256

#define CPU_RAM_CAPACITY 128

#define MAX_POINTERNAME_LEN 128

#define STDOUT_LINE_LEN 150

struct Processor{
    Stack stk;
    Elem_t RAX = 0, RBX = 0, RCX = 0, RDX = 0;
    size_t programm_counter = 0, cs_size = 0;
    unsigned char cs[CPU_CS_CAPACITY] = {};
    int RAM[CPU_RAM_CAPACITY] = {};
};

struct MyFileStruct{
    char* buff = nullptr;
    size_t num_of_str = 0;
    size_t filesize = 0;
};

struct MyPointer{
    char name[MAX_POINTERNAME_LEN] = {};
    ssize_t address = -1;
    char bin_code = 0;
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
            printf("%4d", char_binary_code[j]);                                                                       \
        }                                                                                                             \
        printf("\n");                                                                                                 \
        for(int k = 0; k < i; k++){                                                                                   \
            if(k - i == 1) printf("   ");                                                                             \
            else printf("    ");                                                                                      \
        }                                                                                                             \
        printf("   \033[1;34m^\033[0m\n"); }while(0)

#define STRIKE_ME_OUT() do{ for(size_t trash = 0; trash < STDOUT_LINE_LEN; trash++) printf("–");                      \
        printf("\n"); }while(0)

#define MAKE_COND_JMP(name, operand, binary_code) case binary_code:                                                   \
                int_arg = *(int *)(cpu->cs + cpu->programm_counter);                                                  \
                printf("int_arg = %d\n", int_arg);                                                                    \
                if(int_arg < 0){                                                                                      \
                    printf("\033[1;31mError\033[0m: Unknown pointer address.\n");                                     \
                    return -1;                                                                                        \
                }                                                                                                     \
                                                                                                                      \
                printf("\033[1;32mCase " name "\033[0m\n");                                                           \
                printf("int_arg = %d\n", int_arg);                                                                    \
                fprintf(logfile, "Case " name "\n");                                                                  \
                fprintf(logfile, "int_arg = %d\n", int_arg);                                                          \
                if(StackPop(stk, logfile, &second_operand)){                                                          \
                    printf("Bad second pop!\n");                                                                      \
                }                                                                                                     \
                if(StackPop(stk, logfile, &first_operand)){                                                           \
                    printf("Bad first pop!\n");                                                                       \
                }                                                                                                     \
                                                                                                                      \
                if(first_operand operand second_operand){                                                             \
                    fprintf(logfile, "Yes!\n");                                                                       \
                    cpu->programm_counter = (size_t)int_arg;                                                          \
                }else{                                                                                                \
                    fprintf(logfile, "No!\n");                                                                        \
                    cpu->programm_counter += sizeof(int);                                                             \
                }                                                                                                     \
                break;                                                                                                \

#define OPCODE_CASE(bit_command) case bit_command:                                                                    \
                    printf("\n" #bit_command " case.\n");                                                             \
                    break;                                                                                            \

#define IS_JUMP(curr_command) !strcmp(curr_command, "jb") || !strcmp(curr_command, "jmp") ||                          \
 !strcmp(curr_command, "ja") || !strcmp(curr_command, "jae") || !strcmp(curr_command, "jbe") ||                       \
 !strcmp(curr_command, "je") || !strcmp(curr_command, "jne") || !strcmp(curr_command, "call") ||                      \
 !strcmp(curr_command, "ret")

#define CLEAR_STR(curr_command)         for(size_t better_call_define = 0; better_call_define < MAX_POINTERNAME_LEN;  \
  better_call_define++) curr_command[better_call_define] = '\0';


#define RAM_REG_ASSIGN(stk, cpu, register, int_arg, logfile)    case register:                                        \
                        int_arg = (int)cpu->register;                                                                 \
                        if(int_arg < 0){                                                                              \
                            printf("\033[1;31mError\033[0m: Bad RAM address.\n");                                     \
                            return -1;                                                                                \
                        }                                                                                             \
                        StackPush(stk, logfile, int_arg);                                                             \
                    break;                                                                                            \

#define READ_INT(int_arg, pc) do{ int_arg = *(int *)(cpu->cs + pc); pc += sizeof(int); }while(0)

#define RAM_REG_POP(cpu, register, int_arg, operand, logfile)       case register:                                    \
                        int_arg = (int)cpu->register;                                                                 \
                        if(int_arg < 0){                                                                              \
                            printf("\033[1;31mError\033[0m: Bad RAM address.\n");                                     \
                            return -1;                                                                                \
                        }                                                                                             \
                        cpu->RAM[int_arg] = (int)operand;                                                                  \
                    break;

enum FUNC_CODES{
    CPU_VERSION       = (10),
    NUM_OF_REGS       = (4),
    NUM_OF_COMMANDS   = (17),
    CPU_INIT_CAP      = (10),
    REGISTER_BIT      = SINGLE_BIT(7),
    RPOP              = SINGLE_BIT(3),
    IMREG_BIT         = (0b10000000u),
    REGISTER_BITS     = (0b01100000u),
    COMMAND_BITS      = (0b00011111u)
};

#define MASK_LOWER(bits) ((1U << (bits)) - 1)

enum BIT_FUNC_CODES{
    PUSH     = (0b00000001),
    DIV      = (0b00000010),
    SUB      = (0b00000011),
    POP      = (0b00000100),
    OUT      = (0b00000101),
    IN       = (0b00000110),
    MUL      = (0b00000111),
    SQRT     = (0b00001000),
    ADD      = (0b00001001),
    CALL     = (0b00001010),
    RET      = (0b00001100),
    JMP      = (0b00001101),
    JB       = (0b00001110),
    JA       = (0b00001111),
    JAE      = (0b00010000),
    JBE      = (0b00010001),
    JE       = (0b00010010),
    JNE      = (0b00010011),
    RAMPUSH  = (0b00010100),
    RAMPOP   = (0b00010101),
    RAMPAINT = (0b00010110),
    HLT      = (0b11111111),
    JMPFLG   = (0b11111110),
    CALLFLG  = (0b11111101),
    EOM      = (0b11111100),      // EOM = End of Main
    RAX      = (0b00000000),
    RBX      = (0b00100000),
    RCX      = (0b01000000),
    RDX      = (0b01100000),
};

int read_from_file(const char* filename, MyFileStruct *FileStruct, FILE* logfile);

int string_processing_asm(MyFileStruct* FileStruct, FILE* bin_output, FILE* logfile);

void string_processing_disasm(char* buff, FILE* output, FILE* logfile);

int kernel(Processor *cpu, FILE* logfile, const unsigned char* bin_buff);

int CpuDtor(Processor *cpu, FILE* logfile);

uint32_t CpuVerificator(Processor *cpu, FILE* logfile);

uint32_t CpuDump(Processor *cpu, FILE* logfile);

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile);

unsigned char* read_from_bin_file(const char* filename, FILE* logfile);

int simple_pointer_search(MyPointer *pointers, char *name, int pointers_counter);

bool is_pointer(char* string, size_t len);

bool have_parenthes(char* arg, size_t len);

#include "main_header.hpp"
#include "stack_funcs.cpp"

char* read_from_file(char* filename, FILE* logfile){
    FILE* input_file = fopen(filename, "r");
    struct stat filestat;
    size_t size = 0;

    stat(filename, &filestat);
    size = filestat.st_size;

    char *buff = (char*)calloc(size + 1, sizeof(char));

    if(fread(buff, 1, size + 1, input_file) < size){
        fprintf(logfile, "fread readed less than size!\n");
    }

    buff[size] = '\0';

    fclose(input_file);
    return buff;
}

int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile){
    CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    CPU_VERIF(logfile == nullptr, "logfile is nullptr!");
    CPU_VERIF(capacity == 0, "capacity is zero!");

    StackCtor(&(cpu->stk), capacity, logfile);
    cpu->RAX = 0;
    cpu->RBX = 0;
    cpu->RCX = 0;
    cpu->RDX = 0;
    return 0;
}

int CpuDtor(Processor *cpu, FILE* logfile){
    CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    StackDtor(&(cpu->stk), logfile);
    cpu->RAX = VM_POISON;
    cpu->RBX = VM_POISON;
    cpu->RCX = VM_POISON;
    cpu->RDX = VM_POISON;
    return 0;
}

uint32_t CpuVerificator(Processor *cpu, FILE* logfile){
    // CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    if(cpu == nullptr){
        fprintf(logfile, "[CPU Verificator] cpu is nullptr!\n");
        return SINGLE_BIT(11);
    }
    GENERAL_VERIFICATION(&(cpu->stk), logfile);
    return 0;
}

uint32_t CpuDump(Processor *cpu, FILE* logfile){
    fprintf(logfile, "[CPU DUMP]\nRegisters:\n\trax = %f\n\trbx = %f\n\trcx = %f\n\trdx = %f\nPC: %lu\n",
    cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX, cpu->programm_counter);

    return StackDump(&(cpu->stk), logfile);
}

void string_processing_asm(char* buff, FILE* output, FILE* logfile){
    size_t line_counter = 0, len = 0;
    int int_command = VM_POISON, int_arg = VM_POISON;
    bool silent_arg = false;
    char curr_command[INIT_LEN] = {}, curr_arg[INIT_LEN] = {}, output_str[INIT_LEN] = {}, version[INIT_LEN] = {};

    fprintf(logfile, "Initial buff::\n%s\n", buff);

    sprintf(version, "VERSION: %d\n", CPU_VERSION);

    // fputs(version, output);

    while(buff[1]){
        if(!isalpha(*buff)) SKIP_STR();

        if(sscanf(buff, "%" STRINGIFY(INIT_LEN - 1) "s%n", curr_command, &len) == 0){
            fprintf(logfile, "Failed to read command!\n");
        }

        buff += len;
        fprintf(logfile, "\ncommand: %s, strlen = %zu, buff::%s\n", curr_command, strlen(curr_command), buff);

        if(*buff == '\n' || sscanf(buff, "%s", curr_arg) == 0 || *buff == '\0'){
            fprintf(logfile, "Failed to read argument!\n");
            silent_arg = true;
        }

        fprintf(logfile, "curr_command = %s, curr_arg = %s, Silentness = %d\n", curr_command, curr_arg, silent_arg);

        if(silent_arg){
            COMMAND_COMPARE_ASM("div", DIV, curr_command, int_command);
            COMMAND_COMPARE_ASM("sub", SUB, curr_command, int_command);
            COMMAND_COMPARE_ASM("out", OUT, curr_command, int_command);
            COMMAND_COMPARE_ASM("hlt", HLT, curr_command, int_command);
            COMMAND_COMPARE_ASM("in",  IN, curr_command, int_command);
            COMMAND_COMPARE_ASM("mul", MUL, curr_command, int_command);

            if(int_command == VM_POISON){
                fprintf(logfile, "Error: Unknown command: %s, line %zu\n", curr_command, line_counter);
                continue;
            }

            sprintf(output_str, "%d\n", int_command);
        }
        else{
            fprintf(logfile, "Non-silent case.\n");
            if(atoi(curr_arg) == 0){
                REGISTER_COMPARE_ASM("rax", RAX, curr_arg, int_arg);
                REGISTER_COMPARE_ASM("rbx", RBX, curr_arg, int_arg);
                REGISTER_COMPARE_ASM("rcx", RCX, curr_arg, int_arg);
                REGISTER_COMPARE_ASM("rdx", RDX, curr_arg, int_arg);

                if(int_arg == VM_POISON){
                    fprintf(logfile, "Error: Unknown argument at %zu line: %s\n", line_counter, curr_arg);
                    SKIP_STR();
                    continue;
                }

                COMMAND_COMPARE_ASM("push", RPUSH, curr_command, int_command);
                COMMAND_COMPARE_ASM("pop", POP, curr_command, int_command);
            }
            else{
                COMMAND_COMPARE_ASM("push", PUSH, curr_command, int_command);
                COMMAND_COMPARE_ASM("div", DIV, curr_command, int_command);
                COMMAND_COMPARE_ASM("sub", SUB, curr_command, int_command);
                COMMAND_COMPARE_ASM("out", OUT, curr_command, int_command);
                COMMAND_COMPARE_ASM("hlt", HLT, curr_command, int_command);
                COMMAND_COMPARE_ASM("in",  IN, curr_command, int_command);
                COMMAND_COMPARE_ASM("mul", MUL, curr_command, int_command);

                int_arg = atoi(curr_arg);
            }
            if(int_command == VM_POISON){
                fprintf(logfile, "Error: Command %s can`t be used with argument %s, line %zu\n", curr_command,
                        curr_arg, line_counter);
                continue;
            }
            sprintf(output_str, "%d %d\n", int_command, int_arg);
        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully!\n");
            fprintf(logfile, "Wroten string: %s\n", output_str);
        }

        if(!silent_arg){
            SKIP_STR();
        }
        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;

        line_counter++;
    }
}

void string_processing_disasm(char* buff, FILE* output, FILE* logfile){
    bool silent_arg = false;
    int int_command = VM_POISON, readed_len = VM_POISON;
    char curr_command[INIT_LEN] = {}, output_str[INIT_LEN] = {}, curr_arg[INIT_LEN] = {};

    while(*buff){
        sscanf(buff, "%s%n", curr_command, &readed_len);
        buff += readed_len;
        printf("curr command: %s, strlen + 1 = %d\n", curr_command, readed_len + 1);

        switch(*buff){
            case ' ':
                sscanf(buff, "%s%n", curr_arg, &readed_len);
                buff += readed_len;
                printf("case 1\n");
                while(*buff != '\n') buff++;
                buff++;
                break;
            case '\n':
                *curr_arg = '\0';
                silent_arg = true;
                buff++;
                printf("case 2\n");
                break;
            case '\0':
                printf("case 3\n");
                continue;
            default:
                while(*buff != '\n' && *buff != ' ') buff++;
                buff++;
        }
        printf("curr_arg = %s\n", curr_arg);
        int_command = atoi(curr_command);

        COMMAND_COMPARE_DISASM("push", PUSH, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("push", RPUSH, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("div", DIV, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("sub", SUB, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("out", OUT, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("pop", POP, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("hlt", HLT, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("in" , IN, silent_arg, int_command, curr_command);
        COMMAND_COMPARE_DISASM("mul", MUL, silent_arg, int_command, curr_command);

        printf("\nreaded::%s, %s\n", curr_command, curr_arg);

        if(silent_arg){
            sprintf(output_str, "%s\n", curr_command);
        }
        else{
            if(int_command == RPUSH || int_command == POP){
                switch(atoi(curr_arg)){
                    // [[fallthrough]];
                    COPY_FR_REG(RAX, rax, curr_arg);
                    COPY_FR_REG(RBX, rbx, curr_arg);
                    COPY_FR_REG(RCX, rcx, curr_arg);
                    COPY_FR_REG(RDX, rdx, curr_arg);
                    default:
                        fprintf(logfile, "Error: Unknown register code!\n");
                        break;
                }
            }
            sprintf(output_str, "%s %s\n", curr_command, curr_arg);
        }
        if(atoi(curr_command) == VM_POISON){
            strcpy(output_str, "Unknown machine word.\n");
        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully! Silentness: %d\n", silent_arg);
            fprintf(logfile, "Wroten string: %s\n", output_str);
        }

        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;
    }
}

int kernel(const char* buff, Processor *cpu, FILE* logfile){
    Elem_t first_operand = VM_POISON, second_operand = VM_POISON;
    size_t len = 0;
    char curr_command[INIT_LEN], curr_arg[INIT_LEN];
    Stack* stk = &(cpu->stk);

    while(*buff){
        sscanf(buff, "%s%n", curr_command, &len);
        buff += len;
        // printf("curr command: %s, strlen = %lu\n", curr_command, strlen(curr_command));

        switch(*buff){
            case ' ':
                // printf("case 1\n");
                sscanf(buff, "%s", curr_arg);
                buff += strlen(curr_arg);
                while(*buff != '\n') buff++;
                buff++;
                break;
            case '\n':
                // printf("case 2\n");
                *curr_arg = '\0';
                buff++;
                break;
            case '\0':
                // printf("case 3\n");
                continue;
            default:
                while(*buff != '\n' && *buff != ' ') buff++;
                buff++;
        }
        // printf("curr_arg = %s\n", curr_arg);

        switch(atoi(curr_command)){
            case PUSH:
                #ifdef DEBUG
                printf("Case PUSH\n");
                printf("\ncurr_arg = %s\n", curr_arg);
                #endif

                StackPush(stk, logfile, atoi(curr_arg));
                break;
            case RPUSH:
                #ifdef DEBUG
                printf("Case RPUSH.\n");
                fprintf(logfile, "Case RPUSH.\n");
                #endif

                switch(atoi(curr_arg)){
                    PUSH_FR_REG(stk, logfile, RAX);
                    PUSH_FR_REG(stk, logfile, RBX);
                    PUSH_FR_REG(stk, logfile, RCX);
                    PUSH_FR_REG(stk, logfile, RDX);
                }
                break;
            case POP:
                #ifdef DEBUG
                printf("Case Pop\n");
                fprintf(logfile, "Case Pop\n");
                #endif

                StackPop(stk, logfile, &first_operand);
                switch(atoi(curr_arg)){
                    REG_ASSIGN(cpu, logfile, RAX, first_operand);
                    REG_ASSIGN(cpu, logfile, RBX, first_operand);
                    REG_ASSIGN(cpu, logfile, RCX, first_operand);
                    REG_ASSIGN(cpu, logfile, RDX, first_operand);
                }
                break;
            case DIV:
                #ifdef DEBUG
                printf("Case DIV\n");
                fprintf(logfile, "Case DIV\n");
                #endif

                StackPop(stk, logfile, &second_operand);
                StackPop(stk, logfile, &first_operand);
                if(!IsEqual(second_operand, EPS)){
                    StackPush(stk, logfile, first_operand / second_operand);
                }
                else{
                    printf("Dividing by zero!\n");
                }
                break;
            case SUB:
                #ifdef DEBUG
                printf("Case SUB\n");
                fprintf(logfile, "Case SUB\n");
                #endif

                StackPop(stk, logfile, &second_operand);
                StackPop(stk, logfile, &first_operand);
                StackPush(stk, logfile, first_operand - second_operand);
                break;
            case IN:
                #ifdef DEBUG
                printf("Case IN\n");
                fprintf(logfile, "Case IN\n");
                #endif

                fscanf(stdin, "%lf", &first_operand);
                StackPush(stk, logfile, first_operand);
                break;
            case MUL:
                #ifdef DEBUG
                printf("Case MUL\n");
                fprintf(logfile, "Case MUL\n");
                #endif

                StackPop(stk, logfile, &first_operand);
                StackPop(stk, logfile, &second_operand);
                StackPush(stk, logfile, first_operand * second_operand);
                break;
            case OUT:
                #ifdef DEBUG
                printf("Case OUT\n");
                fprintf(logfile, "Case OUT\n");
                #endif

                StackPop(stk, logfile, &first_operand);
                fprintf(stdout, "\t\t\t\t[OUT]: %f\n", first_operand);
                break;
            case HLT:
                #ifdef DEBUG
                printf("Case HLT\n");
                fprintf(logfile, "Case HLT\n");
                #endif

                fprintf(stdout, "HALT: exiting...\n");
                return 0;
        }

        CpuDump(cpu, logfile);
        cpu->programm_counter++;
    }
    return 0;
}

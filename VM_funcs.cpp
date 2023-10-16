#include "main_header.hpp"
#include "stack_funcs.cpp"

char* read_from_file(char* filename, FILE* logfile){
    FILE* input_file = fopen(filename, "r");
    struct stat filestat;
    size_t size = 0;

    stat(filename, &filestat);
    size = (size_t)filestat.st_size;

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


int* read_from_bin_file(char* filename, FILE* logfile){
    FILE* input_file = fopen(filename, "rb");
    struct stat filestat;
    size_t size = 0;

    stat(filename, &filestat);
    size = (size_t)filestat.st_size / 4;
    printf("size from file: %lu\n", size);

    int *buff = (int*)calloc(size + 1, sizeof(int));

    if(fread(buff, sizeof(int), size + 1, input_file) < size){
        fprintf(logfile, "fread readed less than size!\n");
    }

    buff[size] = VM_POISON;

    fclose(input_file);
    return buff;
}


int kernel(const char* buff, Processor *cpu, FILE* logfile, const int* bin_buff){
    Elem_t first_operand = VM_POISON, second_operand = VM_POISON;
    size_t len = 0;
    char curr_command[INIT_LEN], curr_arg[INIT_LEN];
    int int_command = 0, int_arg = VM_POISON, int_reg = VM_POISON;
    Stack* stk = &(cpu->stk);

    if(bin_buff[0] != CPU_VERSION || bin_buff[1] != NUM_OF_REGS || bin_buff[2] != NUM_OF_COMMANDS){
        fprintf(stdout, "Wrong signature! Exiting...\n");
        return -1;
    }
    cpu->programm_counter += 3; // 3 for cpu version, num of regs and num of commands;

    for(size_t i = 0; i < CPU_CS_SIZE && bin_buff[1] != VM_POISON; i++){
        cpu->cs[i] = *bin_buff;
        bin_buff++;
    }

    for(;cpu->programm_counter < CPU_CS_SIZE || bin_buff[cpu->programm_counter] != VM_POISON;){
        int_command = cpu->cs[cpu->programm_counter++];
        printf("int_command = %d, pc = %d\n", int_command, cpu->programm_counter);

        switch(int_command){
            case RPUSH:
                int_reg = cpu->cs[cpu->programm_counter++];

                #ifdef DEBUG
                printf("\033[1;32mCase RPUSH.\033[0m\n");
                fprintf(logfile, "Case RPUSH.\n");
                fprintf(logfile, "int_reg = %d\n", int_reg);
                printf("int_reg = %d\n", int_reg);
                #endif

                switch(int_reg){
                    PUSH_FR_REG(stk, logfile, RAX);
                    PUSH_FR_REG(stk, logfile, RBX);
                    PUSH_FR_REG(stk, logfile, RCX);
                    PUSH_FR_REG(stk, logfile, RDX);
                }
                CpuDump(cpu, logfile);
                continue;
            case PUSH:
                int_arg = cpu->cs[cpu->programm_counter++];

                #ifdef DEBUG
                printf("\033[1;32mCase Push\033[0m\n");
                printf("int_arg = %d\n", int_arg);
                fprintf(logfile, "Case Push\n");
                fprintf(logfile, "int_arg = %d\n", int_arg);
                #endif

                StackPush(stk, logfile, int_arg);
                break;
            case POP:
                int_arg = cpu->cs[cpu->programm_counter++];
                StackPop(stk, logfile, &first_operand);

                #ifdef DEBUG
                printf("\033[1;32mCase Pop.\033[0m\n");
                fprintf(logfile, "Case Pop\n");
                printf("first_operand = %f\n", first_operand);
                printf("int_arg = %d\n", int_arg);
                #endif

                switch(int_arg){
                    REG_ASSIGN(cpu, logfile, RAX, first_operand);
                    REG_ASSIGN(cpu, logfile, RBX, first_operand);
                    REG_ASSIGN(cpu, logfile, RCX, first_operand);
                    REG_ASSIGN(cpu, logfile, RDX, first_operand);
                }
                CpuDump(cpu, logfile);
                break;
            case DIV:
                #ifdef DEBUG
                printf("\033[1;32mCase DIV\033[0m\n");
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
                printf("\033[1;32mCase SUB.\033[0m\n");
                fprintf(logfile, "Case SUB\n");
                #endif

                StackPop(stk, logfile, &second_operand);
                StackPop(stk, logfile, &first_operand);
                StackPush(stk, logfile, first_operand - second_operand);
                break;
            case IN:
                fscanf(stdin, "%lf", &first_operand);
                StackPush(stk, logfile, first_operand);

                #ifdef DEBUG
                printf("\033[1;32mCase IN.\033[0m\n");
                fprintf(logfile, "Case IN\n");
                printf("\nScanned: %lf\n", first_operand);
                #endif

                break;
            case MUL:
                #ifdef DEBUG
                printf("\033[1;32mCase MUL.\033[0m\n");
                fprintf(logfile, "Case MUL\n");
                #endif

                StackPop(stk, logfile, &first_operand);
                StackPop(stk, logfile, &second_operand);
                StackPush(stk, logfile, first_operand * second_operand);
                break;
            case OUT:
                #ifdef DEBUG
                printf("\033[1;32mCase OUT.\033[0m\n");
                fprintf(logfile, "Case OUT\n");
                #endif

                StackPop(stk, logfile, &first_operand);
                fprintf(stdout, "\t\t\t\t[OUT]: %f\n", first_operand);
                break;
            case HLT:
                #ifdef DEBUG
                printf("\033[1;32mCase HLT.\033[0m\n");
                fprintf(logfile, "Case HLT\n");
                #endif

                fprintf(stdout, "HALT: exiting...\n");
                return 0;
        }
    }

    if(buff == nullptr) return 0;

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


uint32_t CpuDump(Processor *cpu, FILE* logfile){
    size_t space_counter = 0;
    fprintf(logfile, "[CPU DUMP]\nRegisters:\n\tRAX = %f\n\tRBX = %f\n\tRCX = %f\n\tRDX = %f\nPC: %lu\n",
    cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX, cpu->programm_counter);

    fprintf(logfile, "Command segment dump:\n");
    for(size_t i = 0; i < CPU_CS_SIZE && cpu->cs[i + 1] != HLT; i++){
        fprintf(logfile, "%02d ", cpu->cs[i]);
    }
    fprintf(logfile, "\n");

    space_counter = 3 * (cpu->programm_counter);
    for(size_t i = 0; i < space_counter; i++){
        fprintf(logfile, " ");
    }
    fprintf(logfile, "^\n");

    return StackDump(&(cpu->stk), logfile);
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

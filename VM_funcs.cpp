#include "main_header.hpp"
#include "stack_funcs.cpp"


#ifdef NO_BINARY_READ
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
#endif


int CpuCtor(Processor *cpu, size_t capacity, FILE* logfile){
    CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    CPU_VERIF(logfile == nullptr, "logfile is nullptr!");
    CPU_VERIF(capacity == 0, "capacity is zero!");

    if(StackCtor(&(cpu->stk), capacity, logfile) != 0){
        fprintf(logfile, "[CPU Verificator] StackCtor error! \n");
        return -1;
    }
    cpu->RAX = 0;
    cpu->RBX = 0;
    cpu->RCX = 0;
    cpu->RDX = 0;
    return 0;
}


unsigned char* read_from_bin_file(const char* filename, FILE* logfile){
    FILE* input_file = fopen(filename, "rb");
    struct stat filestat;
    size_t size = 0, read_size = 0;

    stat(filename, &filestat);
    size = (size_t)filestat.st_size;
    printf("size from file: %lu\n", size);

    unsigned char *buff = (unsigned char*)calloc(size + 2, sizeof(char));
    read_size = fread(buff, sizeof(char), size, input_file);
    if(read_size < size){
        fprintf(logfile, "fread read less than size!\n");
    }

    buff[read_size] = COMMAND_BITS;

    printf("buff[size] = %d\n", buff[size]);
    printf("Init buff:\n%s\n", buff);

    fclose(input_file);
    return buff;
}


int kernel(Processor *cpu, FILE* logfile, const unsigned char* bin_buff){
    Elem_t first_operand = VM_POISON, second_operand = VM_POISON;
    int int_arg = VM_POISON;
    unsigned char char_command = (char)0, char_reg = (char)0;
    Stack* stk = &(cpu->stk);

    if(bin_buff[0] != CPU_VERSION || bin_buff[1] != NUM_OF_REGS || bin_buff[2] != NUM_OF_COMMANDS){
        fprintf(stdout, "Wrong signature! Exiting...\n");
        return -1;
    }
    cpu->programm_counter += 3; // 3 for cpu version, num of regs and num of commands;

    printf("\t\t\t\033[1;36mGot buffer:\033[0m\n");
    for(int j = 1; bin_buff[j - 1] != COMMAND_BITS; j++){
        printf("%d\n", bin_buff[j]);
    }

    cpu->cs[0] = bin_buff[0];
    cpu->cs[1] = bin_buff[1];
    for(size_t i = 2; i < CPU_CS_SIZE && bin_buff[i - 1] != HLT; i++){
        cpu->cs[i] = bin_buff[i];
    }

    printf("\t\t\t\033[1;36mSaved buffer:\033[0m\n");
    printf("%d\n", cpu->cs[0]);
    for(int j = 1; cpu->cs[j - 1] != HLT; j++){
        printf("%d\n", cpu->cs[j]);
    }

    while(cpu->programm_counter < CPU_CS_SIZE && cpu->cs[cpu->programm_counter - 1] != COMMAND_BITS){
        char_command = cpu->cs[cpu->programm_counter++];
        printf("char_command = %d, pc = %zu\n", char_command, cpu->programm_counter);

        switch(char_command & COMMAND_BITS){
            case PUSH:
                if((char_command & IMREG_BIT) == 0){
                    int_arg = *(int *)(cpu->cs + cpu->programm_counter); // TODO: to define
                    cpu->programm_counter+=4;

                    #ifdef DEBUG
                    printf("\033[1;32mCase Push\033[0m\n");
                    printf("int_arg = %d\n", int_arg);
                    fprintf(logfile, "Case Push\n");
                    fprintf(logfile, "int_arg = %d\n", int_arg);
                    #endif

                    StackPush(stk, logfile, int_arg);
                    break;
                }else{

                    #ifdef DEBUG
                    printf("\033[1;32mCase PUSH.\033[0m\n");
                    fprintf(logfile, "Case PUSH.\n");
                    fprintf(logfile, "char_reg = %d\n", char_reg);
                    printf("char_reg = %d\n", char_reg);
                    #endif

                    switch((char_command & REGISTER_BITS)){
                        PUSH_FR_REG(stk, logfile, RAX);
                        PUSH_FR_REG(stk, logfile, RBX);
                        PUSH_FR_REG(stk, logfile, RCX);
                        PUSH_FR_REG(stk, logfile, RDX);
                    }

                    break;
                }
            case POP:
                #ifdef DEBUG
                printf("\033[1;32mCase Pop.\033[0m\n");
                fprintf(logfile, "Case Pop\n");
                printf("first_operand = %f, char_command: %d\n", first_operand, (char_command & REGISTER_BITS) >> 5);
                printf("int_arg = %d\n", int_arg);
                #endif

                StackPop(stk, logfile, &first_operand);

                switch((char_command & REGISTER_BITS)){
                    REG_ASSIGN(cpu, logfile, RAX, first_operand);
                    REG_ASSIGN(cpu, logfile, RBX, first_operand);
                    REG_ASSIGN(cpu, logfile, RCX, first_operand);
                    REG_ASSIGN(cpu, logfile, RDX, first_operand);
                }
                printf("\033[1;34mREGS\033[0m: %f %f %f %f\n", cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX);

                break;
            MAKE_COND_JMP("JMP", =, JMP);
            MAKE_COND_JMP("JB", <, JB);
            MAKE_COND_JMP("JA", >, JA);
            MAKE_COND_JMP("JAE", >=, JAE);
            MAKE_COND_JMP("JBE", >=, JBE);
            MAKE_COND_JMP("JE", ==, JE);
            MAKE_COND_JMP("JNE", !=, JNE);
            case DIV:
                #ifdef DEBUG
                printf("\033[1;32mCase DIV\033[0m\n");
                fprintf(logfile, "Case DIV\n");
                #endif

                if(StackPop(stk, logfile, &second_operand)){
                    printf("Bad second pop!\n");
                }
                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad first pop!\n");
                }

                if(!IsEqual(second_operand, 0)){
                    StackPush(stk, logfile, first_operand / second_operand);
                }
                else{
                    printf("Dividing by zero!\n");
                    fprintf(logfile, "Dividing by zero!\n");
                    return -1;
                }
                break;
            case SUB:
                #ifdef DEBUG
                printf("\033[1;32mCase SUB.\033[0m\n");
                fprintf(logfile, "Case SUB\n");
                #endif

                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad second pop!\n");
                }
                if(StackPop(stk, logfile, &second_operand)){
                    printf("Bad first pop!\n");
                }

                StackPush(stk, logfile, first_operand - second_operand);
                break;
            case IN:
                #ifdef DEBUG
                printf("\033[1;32mCase IN.\033[0m\n");
                fprintf(logfile, "Case IN\n");
                printf("\nScanned: %lf\n", first_operand);
                #endif

                while(!fscanf(stdin, "%lf", &first_operand)){
                    while(!getchar());
                }

                StackPush(stk, logfile, first_operand);

                break;
            case MUL:
                #ifdef DEBUG
                printf("\033[1;32mCase MUL.\033[0m\n");
                fprintf(logfile, "Case MUL\n");
                #endif

                if(StackPop(stk, logfile, &second_operand)){
                    printf("Bad second pop!\n");
                }
                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad first pop!\n");
                }

                StackPush(stk, logfile, first_operand * second_operand);
                break;
            case OUT:
                #ifdef DEBUG
                printf("\033[1;32mCase OUT.\033[0m\n");
                fprintf(logfile, "Case OUT\n");
                #endif
                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad first pop!\n");
                }
                fprintf(stdout, "\t\t\t\t[OUT]: %f\n", first_operand);
                break;
            case (HLT & COMMAND_BITS):
                #ifdef DEBUG
                printf("\033[1;32mCase HLT.\033[0m\n");
                fprintf(logfile, "Case HLT\n");
                #endif

                fprintf(stdout, "HALT: exiting...\n");
                return 0;
        }

    CpuDump(cpu, logfile);

    }


   /* #ifdef NO_BINARY_READ
    size_t len = 0;
    char curr_command[INIT_LEN], curr_arg[INIT_LEN];
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
    #endif
    */

    return 0;
}


uint32_t CpuDump(Processor *cpu, FILE* logfile){
    size_t space_counter = 0;
    fprintf(logfile, "[CPU DUMP]\nRegisters:\n\tRAX = %f\n\tRBX = %f\n\tRCX = %f\n\tRDX = %f\nPC: %lu\n",
    cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX, cpu->programm_counter);

    fprintf(logfile, "Command segment dump:\n");
    for(size_t i = 0; i + 1 < CPU_CS_SIZE && cpu->cs[i] != HLT; i++){
        fprintf(logfile, "%03d ", cpu->cs[i]);
    }
    fprintf(logfile, "\n");

    space_counter = 4 * (cpu->programm_counter);
    for(size_t i = 0; i < space_counter; i++){
        fprintf(logfile, " ");
    }
    fprintf(logfile, "^\n");

    return StackDump(&(cpu->stk), logfile);
}


int CpuDtor(Processor *cpu, FILE* logfile){
    CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    StackDtor(&(cpu->stk), logfile);

    for(size_t i = 0; i < CPU_CS_SIZE; i++){
        cpu->cs[i] = HLT;
    }

    cpu->RAX = VM_POISON;
    cpu->RBX = VM_POISON;
    cpu->RCX = VM_POISON;
    cpu->RDX = VM_POISON;
    return 0;
}

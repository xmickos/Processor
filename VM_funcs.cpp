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
    for(size_t i = 2; i < CPU_CS_CAPACITY && bin_buff[i - 1] != HLT; i++){
        cpu->cs[i] = bin_buff[i];
        cpu->cs_size++;
    }

    printf("\t\t\t\033[1;36mSaved buffer:\033[0m\n");
    printf("%d\n", cpu->cs[0]);
    for(int j = 1; cpu->cs[j - 1] != HLT; j++){
        printf("%d\n", cpu->cs[j]);
    }

    while(cpu->programm_counter < CPU_CS_CAPACITY && cpu->cs[cpu->programm_counter - 1] != COMMAND_BITS){
        char_command = cpu->cs[cpu->programm_counter++];
        printf("char_command = %d, pc = %zu\n", char_command, cpu->programm_counter);

        switch(char_command & COMMAND_BITS){
            case PUSH:
                if((char_command & IMREG_BIT) == 0){
                    READ_INT(int_arg, cpu->programm_counter);

                    #ifdef DEBUG
                    printf("\033[1;32mCase Push\033[0m\n");
                    printf("int_arg = %d\n", int_arg);
                    fprintf(logfile, "Case Push\n");
                    fprintf(logfile, "int_arg = %d\n", int_arg);
                    #endif

                    StackPush(stk, logfile, int_arg);
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

                }
            break;
            case POP:
                #ifdef DEBUG
                printf("\033[1;32mCase Pop.\033[0m\n");
                fprintf(logfile, "Case Pop\n");
                printf("first_operand = %f, char_command: %d\n", first_operand, (char_command & REGISTER_BITS) >> 5);
                printf("int_arg = %d\n", int_arg);
                #endif

                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad first pop!\n");
                }

                switch((char_command & REGISTER_BITS)){
                    REG_ASSIGN(cpu, logfile, RAX, first_operand);
                    REG_ASSIGN(cpu, logfile, RBX, first_operand);
                    REG_ASSIGN(cpu, logfile, RCX, first_operand);
                    REG_ASSIGN(cpu, logfile, RDX, first_operand);
                }
                printf("\033[1;34mREGS\033[0m: %f %f %f %f\n", cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX);

            break;
            case RAMPOP:

                if(StackPop(stk, logfile, &first_operand)){
                    printf("Bad first pop!\n");
                }

                if(first_operand < 0){
                    printf("\033[1;31mError\033[0m: Bad RAM address.\n");
                    return -1;
                }

                printf("\033[1;32mCase RAM POP\033[0m\n");
                fprintf(logfile, "Case RAM POP\n");

                if((char_command & IMREG_BIT) > 0){
                    printf("imreg bit == 1!\n");
                    switch(char_command & REGISTER_BITS){
                    RAM_REG_POP(cpu, RAX, int_arg, first_operand, logfile);
                    RAM_REG_POP(cpu, RBX, int_arg, first_operand, logfile);
                    RAM_REG_POP(cpu, RCX, int_arg, first_operand, logfile);
                    RAM_REG_POP(cpu, RDX, int_arg, first_operand, logfile);
                    };
                }else{
                    printf("cpu->pc = %zu\n", cpu->programm_counter);
                    printf("cpu->cs[cpu->pc + 0] = %d\n", cpu->cs[cpu->programm_counter + 0]);
                    printf("cpu->cs[cpu->pc + 1] = %d\n", cpu->cs[cpu->programm_counter + 1]);
                    printf("cpu->cs[cpu->pc + 2] = %d\n", cpu->cs[cpu->programm_counter + 2]);
                    printf("cpu->cs[cpu->pc + 3] = %d\n", cpu->cs[cpu->programm_counter + 3]);
                    printf("cpu->cs[cpu->pc + 4] = %d\n", cpu->cs[cpu->programm_counter + 4]);
                    printf("cpu->cs[cpu->pc + 5] = %d\n", cpu->cs[cpu->programm_counter + 5]);
                    printf("cpu->cs[cpu->pc + 6] = %d\n", cpu->cs[cpu->programm_counter + 6]);


                    READ_INT(int_arg, cpu->programm_counter);
                    printf("int_arg after reading from cs is %d\n", int_arg);
                    cpu->RAM[int_arg] = first_operand;
                }
            break;
            case RAMPUSH:

                printf("\033[1;32mCase RAM PUSH\033[0m\n");
                printf("int_arg = %d\n", int_arg);
                fprintf(logfile, "Case RAM PUSH\n");
                fprintf(logfile, "int_arg = %d\n", int_arg);

                if((char_command & IMREG_BIT) == 1){
                    switch(char_command & REGISTER_BITS){
                    RAM_REG_ASSIGN(stk, cpu, RAX, int_arg, logfile);
                    RAM_REG_ASSIGN(stk, cpu, RBX, int_arg, logfile);
                    RAM_REG_ASSIGN(stk, cpu, RCX, int_arg, logfile);
                    RAM_REG_ASSIGN(stk, cpu, RDX, int_arg, logfile);
                    };
                }
                else{
                    READ_INT(int_arg, cpu->programm_counter);
                    printf("int_arg = %d\n", int_arg);
                    if(int_arg < 0){
                        printf("\033[1;31mError\033[0m: Bad RAM address.\n");
                        return -1;
                    }
                    StackPush(stk, logfile, cpu->RAM[int_arg]);
                }
            break;
            case RAMPAINT:

                printf("\033[1;32mCase RAM PAINT\033[0m\n");
                fprintf(logfile, "Case RAM PAINT\n");

                printf("RAM PAINT: :)))\n\n");

                for(int i = 0; i < 10; i++){
                    for(int j = 0; j < 10 && (i * 10 + j < CPU_RAM_CAPACITY); j++){
                        if(cpu->RAM[i * 10 + j] < 1) printf(".");
                        else printf("@");
                    }
                    printf("\n");
                }
                printf("\n");

            break;
            case JMP:
                READ_INT(int_arg, cpu->programm_counter);
                printf("int_arg = %d\n", int_arg);
                if(int_arg < 0){
                    printf("\033[1;31mError\033[0m: Unknown pointer address.\n");
                    return -1;
                }

                cpu->programm_counter = (size_t)int_arg;

                printf("\033[1;32mCase JMP\033[0m\n");
                printf("int_arg = %d\n", int_arg);
                fprintf(logfile, "Case JMP\n");
                fprintf(logfile, "int_arg = %d\n", int_arg);
                break;
            MAKE_COND_JMP("JB",  <,  JB);
            MAKE_COND_JMP("JA",  >,  JA);
            MAKE_COND_JMP("JAE", >=, JAE);
            MAKE_COND_JMP("JBE", <=, JBE);
            MAKE_COND_JMP("JE",  ==, JE);
            MAKE_COND_JMP("JNE", !=, JNE);
            case CALL:
                READ_INT(int_arg, cpu->programm_counter);
                printf("int_arg = %d\n", int_arg);
                if(int_arg < 0){
                    printf("\033[1;31mError\033[0m: Unknown pointer address.\n");
                    return -1;
                }
                cpu->programm_counter = (size_t)int_arg;

                // TODO: VVVVVVV
                // call   -> push (ip + sizeof(char)) + jmp to label
                // return -> ip := pop
                // TODO: memory RAM

                printf("\033[1;32mCase CALL \033[0m\n");
                printf("int_arg = %d\n", int_arg);
                fprintf(logfile, "Case CALL\n");
                fprintf(logfile, "int_arg = %d\n", int_arg);


            break;
            case RET:
                if(StackPop(stk, logfile, &second_operand)){
                    printf("Bad ret pop!\n");
                }

                printf("\033[1;32mCase RET \033[0m\n");
                printf("int_arg = %d\n", int_arg);
                fprintf(logfile, "Case RET\n");
                fprintf(logfile, "int_arg = %d\n", int_arg);

                cpu->programm_counter = (size_t)second_operand;

            break;
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
            case ADD:
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

                StackPush(stk, logfile, second_operand + first_operand);
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

                StackPush(stk, logfile, second_operand - first_operand);
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
            case (EOM & COMMAND_BITS):
                #ifdef DEBUG
                printf("\033[1;32mCase End of Main.\033[0m\n");
                fprintf(logfile, "Case End of Main\n");
                #endif

                fprintf(stdout, "End of Main. Exiting...\n");
                return 0;
        }

    CpuDump(cpu, logfile);

    }

    return 0;
}


uint32_t CpuDump(Processor *cpu, FILE* logfile){
    size_t line_size = 107;
    fprintf(logfile, "[CPU DUMP]\nRegisters:\n\tRAX = %f\n\tRBX = %f\n\tRCX = %f\n\tRDX = %f\nPC: %lu\n",
    cpu->RAX, cpu->RBX, cpu->RCX, cpu->RDX, cpu->programm_counter);

    fprintf(logfile, "Command segment dump:\n");

    size_t i = 0, curr_code_segment = 0;

    while(i < cpu->cs_size && cpu->cs[i] != HLT){

        if((cpu->programm_counter) >= (curr_code_segment * 25) &&
           (cpu->programm_counter) < (curr_code_segment + 1) * 25){

            fprintf(logfile, "––> ");
            for(;i < ((curr_code_segment + 1) * (line_size - 7)) / 4; i++) fprintf(logfile, "%03d ", cpu->cs[i]);
            fprintf(logfile, "\n");
        }
        else{
            fprintf(logfile, "    ");
            for(;i < ((curr_code_segment + 1) * (line_size - 7)) / 4; i++) fprintf(logfile, "%03d ", cpu->cs[i]);
            fprintf(logfile, "\n");
        }
        curr_code_segment++;
    }
    for(size_t j = 0; j < (cpu->programm_counter % 25) + 1; j++) fprintf(logfile, "    ");
    fprintf(logfile, " ^\n");

    fprintf(logfile, "RAM dump:\n");
    for(int k = 0; k < CPU_RAM_CAPACITY;){
        for(int j = 0; j < 28 && k + j < CPU_RAM_CAPACITY; j++){
            fprintf(logfile, "%03d ", cpu->RAM[k + j]);
        }
        fprintf(logfile, "\n");
        k += 28;
    }
    fprintf(logfile, "\n");

    return StackDump(&(cpu->stk), logfile);
}


int CpuDtor(Processor *cpu, FILE* logfile){
    CPU_VERIF(cpu == nullptr, "cpu is nullptr!");
    StackDtor(&(cpu->stk), logfile);

    for(size_t i = 0; i < CPU_CS_CAPACITY; i++){
        cpu->cs[i] = HLT;
    }

    cpu->RAX = VM_POISON;
    cpu->RBX = VM_POISON;
    cpu->RCX = VM_POISON;
    cpu->RDX = VM_POISON;
    return 0;
}

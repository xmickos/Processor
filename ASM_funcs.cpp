#include "main_header.hpp"
#include "stack_funcs.cpp"

int read_from_file(const char* filename, MyFileStruct* FileStruct, FILE* logfile){
    FILE* input_file = fopen(filename, "r");
    struct stat filestat;

    stat(filename, &filestat);
    FileStruct->filesize = (size_t)filestat.st_size;

    FileStruct->buff = (char*)calloc(FileStruct->filesize + 1, sizeof(char));
    if(FileStruct->buff == nullptr){
        printf("\033[1;31mError\033[0m: Bad calloc!\nExiting...\n");
        return -1;
    }


    if(fread(FileStruct->buff, sizeof(char), FileStruct->filesize, input_file) < FileStruct->filesize){
        fprintf(logfile, "\033[1;31mError\033[0m: fread readed less than size!\n");
        printf("\033[1;31mError\033[0m: fread readed less than size!\nExiting...\n");
        return -1;
    }
    fclose(input_file);

    for(size_t i = 0; i < FileStruct->filesize; i++){
        if(FileStruct->buff[i] == '\n' || FileStruct->buff[i] == '0') FileStruct->num_of_str++;
    }
    printf("Filesize: %lu, num of strings: %lu \n", FileStruct->filesize, FileStruct->num_of_str);

    FileStruct->buff[FileStruct->filesize] = '\0';

    return 0;
}

int string_processing_asm(MyFileStruct* FileStruct, FILE* output, FILE* bin_output, FILE* logfile){
    char* buff = FileStruct->buff, *ref_buff = FileStruct->buff;
    size_t line_counter = 0, len = 0, binary_pos_counter = 0; /*pointers_counter = 0;*/
    int int_arg = VM_POISON, pointers[CPU_CS_SIZE] = {};
    bool silent_arg = false;
    char curr_command[INIT_LEN] = {}, curr_arg[INIT_LEN] = {}, output_str[INIT_LEN] = {}, version[3 * INIT_LEN] = {};

    unsigned char opcode = (char)0u;
    unsigned char *char_binary_code = nullptr;

    for(size_t i = 0; i < CPU_CS_SIZE; i++){
        pointers[i] = -1;
    }


    fprintf(logfile, "Initial buff:\n%s\n", buff);

    sprintf(version, "Current signature: VERSION: %d, %d REGISTERS, %d COMMANDS\n", CPU_VERSION, NUM_OF_REGS, NUM_OF_COMMANDS);
    fputs(version, output);

    char_binary_code = (unsigned char*)calloc(2 * (FileStruct->num_of_str) + 4, sizeof(char));

    // + 4 = 1 + cpu_version + num_of_regs + command_count

    char_binary_code[binary_pos_counter++] = (char)CPU_VERSION;
    char_binary_code[binary_pos_counter++] = (char)NUM_OF_REGS;
    char_binary_code[binary_pos_counter++] = (char)NUM_OF_COMMANDS;

    while(buff[1]){
        opcode = (char)0;
        // while(!isalpha(*buff) && (*buff != ':')) buff++;

        if(sscanf(buff, "%4s%n", curr_command, &len) == 0){         // 4 == STRINGIFY(INIT_LEN - 1)
            printf("\033[1;31mError\033[0m: Unknown command at %zu line. \nExiting...\n", line_counter + 1);
            return -1;
        }

        buff += len;
        fprintf(logfile, "\nCommand: %s, buff:%s\n", curr_command, buff);

        if(*buff == '\n' || sscanf(buff, "%s", curr_arg) == 0 || *buff == '\0'){
            fprintf(logfile, "Failed to read argument!\n");
            silent_arg = true;
        }

        if(curr_command[1] == ':' && curr_command[0] - '9' <= 0 && curr_command[0] - '0' >= 0){
            CHECKPOINT("HERE!!!");
            printf("curr_command: %s, atoi(curr_command): %d\n", curr_command, atoi(curr_command));
            pointers[atoi(curr_command)] = (int)line_counter;
            fprintf(logfile, "Processed as :%d – read: %s\n", atoi(curr_command), curr_command);
            printf("Written: %d\n", pointers[atoi(curr_command)]);
            line_counter++;
            SKIP_STR();
            silent_arg = false;
            continue;
        }

        fprintf(logfile, "curr_command = %s, curr_arg = %s, Silentness = %d\n", curr_command, curr_arg, silent_arg);

        CHECKPOINT("first check");

        printf("curr command: %s\n", curr_command);

        BITWISE_COMPARE_ASM(curr_command, "push", BPUSH, opcode);
        BITWISE_COMPARE_ASM(curr_command, "pop", BPOP, opcode);
        BITWISE_COMPARE_ASM(curr_command, "jmp", BJMP, opcode);

        if((opcode & COMMAND_BITS) != 0u){

            CHECKPOINT("non-poison - case");

            if(silent_arg == true){
                fprintf(logfile, "\033[1;31mError\033[0m: Wrong command at %zu line.\nExiting...\n", line_counter + 1);
                return -1;
            }


            if(curr_arg[0] == 'r' && curr_arg[2] == 'x' && abs(curr_arg[1] - 'a') <= 3){
                fprintf(logfile, "curr arg: %s, register case.\n", curr_arg);
                printf("Curr arg: %s, register case\n", curr_arg);

                BITWISE_COMPARE_ASM(curr_arg, "rax", BRAX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rbx", BRBX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rcx", BRCX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rdx", BRDX, opcode);

                printf("opcode after regs is %d\n", opcode);
                fprintf(logfile, "opcode after regs is %d\n", opcode);


                opcode |= REGISTER_BIT;
                char_binary_code[binary_pos_counter++] = opcode;

                printf("full opcode after regs is %d\n", opcode);
                fprintf(logfile, "full opcode after regs is %d\n", opcode);

            }else{
                if(sscanf(curr_arg, "%d", &int_arg) != 1 && curr_arg[0] != ':'){
                    printf("\033[1;31mError\033[0m: Bad argument at %zu line.\nExiting...\n", line_counter + 1);
                    return -1;
                }
                CHECKPOINT("IMREG_BIT");
                opcode &= ~IMREG_BIT;
                char_binary_code[binary_pos_counter++] = opcode;
                if(opcode == BPUSH){
                    WRITE_INT(binary_pos_counter, int_arg);
                    binary_pos_counter+=4;
                }
            }

            printf("\033[1;36mOpcode\033[0m: %d\n", opcode);
            if(opcode == BJMP){
                CHECKPOINT("BMJP, ");
                printf("opcode: %d\n", opcode);
                WRITE_INT(binary_pos_counter, JMPFLG);
                binary_pos_counter+=4;
            }
        }
        else{
            CHECKPOINT("after regs");
            printf("FPU after regs: %d\n", opcode);
            fprintf(logfile, "FPU after regs: %d\n", opcode);

            BITWISE_COMPARE_ASM(curr_command, "div", BDIV, opcode);
            BITWISE_COMPARE_ASM(curr_command, "sub", BSUB, opcode);
            BITWISE_COMPARE_ASM(curr_command, "out", BOUT, opcode);
            BITWISE_COMPARE_ASM(curr_command, "hlt", BHLT, opcode);
            BITWISE_COMPARE_ASM(curr_command, "in",  BIN, opcode);
            BITWISE_COMPARE_ASM(curr_command, "mul", BMUL, opcode);

            char_binary_code[binary_pos_counter++] = opcode;

            printf("FPU after all: %d\n", opcode);
            fprintf(logfile, "FPU after all: %d\n", opcode);

            if((opcode & COMMAND_BITS) == 0b0){
                printf("\033[1;31mError\033[0m: Unknown command at %zu line: %s \nExiting...\n", line_counter + 1, curr_command);
                return -1;
            }

            CHECKPOINT("After all command compares.");
        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully!\n");
            fprintf(logfile, "Written string: %s\n", output_str);
        }

        if(!silent_arg){
            SKIP_STR();
        }

        fprintf(logfile, "New buff:%s\n", buff);
        silent_arg = false;

        line_counter++;
        int_arg = VM_POISON;
    }


    printf("\nchar_binary_code: binary_pos_counter = %zu\n", binary_pos_counter);
    for(size_t i = 0; i < binary_pos_counter; i++){
        printf("%3d ", char_binary_code[i]);
    }
    printf("\n");

    buff = ref_buff;

    for(int i = 3; buff[1];){
        if(sscanf(buff, "%s%n", curr_command, &len) && curr_command[1] != ':'){
            i++;
        }

        printf("\ncurr_command: %s, len = %zu, i = %d\n", curr_command, len, i);
        buff += len;

        if(*buff == ' '){
            printf("yep.");
            if(!strcmp(curr_command, "jmp")){
                buff++;

                if(sscanf(buff, "%d", &int_arg) < 1 || int_arg < 0 || int_arg > 10){
                    printf("\033[1;31mError\033[0m: Wrong pointer for jmp.\n");
                    return -1;
                }

                printf("curr_arg:%s, pointers[%d] = %d, i = %d\n", curr_arg, int_arg, pointers[int_arg], i);
                WRITE_INT(i, pointers[int_arg] + 3);
                i+=4;
            }
            buff++;

            if(sscanf(buff, "%d", &int_arg)){
                i+=4;
            }

            SKIP_STR();
        }
    }



    printf("pointers:\n");
    for(size_t i = 0; i < binary_pos_counter; i++){
        printf("%3d", pointers[i]);
    }

    printf("\nchar_binary_code: binary_pos_counter = %zu\n", binary_pos_counter);
    for(size_t i = 0; i < binary_pos_counter; i++){
        printf("%3d ", char_binary_code[i]);
    }


    if(fwrite(char_binary_code, sizeof(char), binary_pos_counter + 2, bin_output) < binary_pos_counter){
        printf("fwrite failed!\n");
        return -1;
    }

    free(char_binary_code);
    return 0;
}


/*     Old processing:
while(buff[1]){
        if(!isalpha(*buff)) SKIP_STR();

        if(sscanf(buff, "%4s%n", curr_command, &len) == 0){         // 4 = STRINGIFY(INIT_LEN - 1)
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
            binary_code[binary_pos_counter++] = int_command;

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
            // TODO: read operation -> print to binary -> check if needs and has arguments -> error/no err
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
            binary_code[binary_pos_counter++] = int_command;
            binary_code[binary_pos_counter++] = int_arg;

        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully!\n");
            fprintf(logfile, "Written string: %s\n", output_str);
        }

        if(!silent_arg){
            SKIP_STR();
        }
        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;

        line_counter++;
    }
*/

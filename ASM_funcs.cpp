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

int simple_pointer_search(MyPointer *pointers, char *name, int pointers_counter){

    for(int i = 0; i < pointers_counter; i++){
        printf("i = %d\n", i);
        if(!strcmp(pointers[i].name, name)){
            return i;
        }
    }

    return -1;
}

int string_processing_asm(MyFileStruct* FileStruct, FILE* output, FILE* bin_output, FILE* logfile){
    char* buff = FileStruct->buff, *ref_buff = FileStruct->buff;
    size_t line_counter = 0, len = 0, binary_pos_counter = 0, pointers_counter = 0;
    int int_arg = VM_POISON;
    MyPointer pointers[CPU_CS_SIZE] = {};
    bool silent_arg = false;
    char curr_command[MAX_POINTERNAME_LEN] = {}, curr_arg[MAX_POINTERNAME_LEN] = {},
         output_str[INIT_LEN] = {}, version[3 * INIT_LEN] = {};

    unsigned char opcode = (char)0u;
    unsigned char *char_binary_code = nullptr;


    fprintf(logfile, "Initial buff:\n%s\n", buff);

    sprintf(version, "Current signature: VERSION: %d, %d REGISTERS, %d COMMANDS\n", CPU_VERSION, NUM_OF_REGS, NUM_OF_COMMANDS);
    fputs(version, output);

    char_binary_code = (unsigned char*)calloc(2 * (FileStruct->num_of_str) + 4, sizeof(char));

    // + 4 = 1 + cpu_version + num_of_regs + command_count

    char_binary_code[binary_pos_counter++] = (char)CPU_VERSION;
    char_binary_code[binary_pos_counter++] = (char)NUM_OF_REGS;
    char_binary_code[binary_pos_counter++] = (char)NUM_OF_COMMANDS;

    while(buff[1]){
        opcode = 0;

        POINTERS_DUMP(pointers, pointers_counter);

        if(sscanf(buff, "%s%n", curr_command, &len) == 0){         // 4 == STRINGIFY(INIT_LEN - 1)
            printf("\033[1;31mError\033[0m: Unknown command at %zu line. \nExiting...\n", line_counter + 1);
            return -1;
        }

        buff += len;
        fprintf(logfile, "\nCommand: %s, buff:%s\n", curr_command, buff);

        if(*buff == '\n' || sscanf(buff, "%s%n", curr_arg, &len) == 0 || *buff == '\0'){
            fprintf(logfile, "Failed to read argument!\n");
            fprintf(stdout, "Failed to read argument!\n");
            silent_arg = true;
        }

        if(curr_command[len - 1] == ':'){
            CHECKPOINT("HERE!!!");
            printf("curr_command: %s, pointers_counter = %zu\n", curr_command, pointers_counter);
            curr_command[len - 1] = '\0';
            pointers[pointers_counter].address = (int)binary_pos_counter;
            strcpy(pointers[pointers_counter].name, curr_command);
            pointers_counter++;
            line_counter++;

            printf("Written: (%s, %zu) \n", pointers[pointers_counter - 1].name,
            pointers[pointers_counter - 1].address);


            printf("pointers:\n");
            for(size_t i = 0; i < pointers_counter; i++){
                printf("(%s, %3zd)\n", pointers[i].name, pointers[i].address);
            }

            SKIP_STR();
            silent_arg = false;
            continue;
        }

        fprintf(logfile, "curr_command = %s, curr_arg = %s, Silentness = %d\n", curr_command, curr_arg, silent_arg);
        fprintf(stdout, "curr_command = %s, curr_arg = %s, Silentness = %d\n", curr_command, curr_arg, silent_arg);

        CHECKPOINT("first check");

        printf("curr command: %s, Silentness: %d\n", curr_command, silent_arg);

        BITWISE_COMPARE_ASM(curr_command, "push", PUSH, opcode);
        BITWISE_COMPARE_ASM(curr_command, "pop",  POP,  opcode);
        BITWISE_COMPARE_ASM(curr_command, "jmp",  JMP,  opcode);
        BITWISE_COMPARE_ASM(curr_command, "jb",  JB,  opcode)

        if((opcode & COMMAND_BITS) != 0u){

            CHECKPOINT("non-poison - case");

            if(silent_arg == true){
                fprintf(logfile, "\033[1;31mError\033[0m: Wrong command at %zu line.\nExiting...\n", line_counter + 1);
                return -1;
            }


            if(curr_arg[0] == 'r' && curr_arg[2] == 'x' && abs(curr_arg[1] - 'a') <= NUM_OF_REGS - 1){
                fprintf(logfile, "curr arg: %s, register case.\n", curr_arg);
                printf("Curr arg: %s, register case\n", curr_arg);

                BITWISE_COMPARE_ASM(curr_arg, "rax", RAX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rbx", RBX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rcx", RCX, opcode);
                BITWISE_COMPARE_ASM(curr_arg, "rdx", RDX, opcode);

                printf("opcode after regs is %d\n", opcode);
                fprintf(logfile, "opcode after regs is %d\n", opcode);


                opcode |= REGISTER_BIT;
                char_binary_code[binary_pos_counter++] = opcode;

                printf("full opcode after regs is %d\n", opcode);
                fprintf(logfile, "full opcode after regs is %d\n", opcode);

            }else{
                printf("curr_arg = %s, len = %zu\n", curr_arg, len);
                if(sscanf(curr_arg, "%d", &int_arg) != 1 && curr_arg[len - 2] != ':'){
                    printf("\033[1;31mError\033[0m: Bad argument at %zu line.\nExiting...\n", line_counter + 1);
                    return -1;
                }
                CHECKPOINT("IMREG_BIT");
                opcode &= ~IMREG_BIT;
                char_binary_code[binary_pos_counter++] = opcode;
                if(opcode == PUSH){
                    printf("My intarg is %d\n", int_arg);
                    WRITE_INT(binary_pos_counter, int_arg);
                    binary_pos_counter += sizeof(int);
                }
            }

            printf("\033[1;36mOpcode\033[0m: %d\n", opcode);
            if(opcode == JMP || opcode == JB){
                CHECKPOINT("BMJP || JB, ");
                if(opcode == JB) printf("(JB)\n");
                printf("opcode: %d\n", opcode);
                WRITE_INT(binary_pos_counter, JMPFLG);
                binary_pos_counter += sizeof(int);
            }

        }
        else{
            CHECKPOINT("after regs");
            printf("FPU after regs: %d\n", opcode);
            fprintf(logfile, "FPU after regs: %d\n", opcode);

            BITWISE_COMPARE_ASM(curr_command, "div", DIV, opcode);
            BITWISE_COMPARE_ASM(curr_command, "sub", SUB, opcode);
            BITWISE_COMPARE_ASM(curr_command, "out", OUT, opcode);
            BITWISE_COMPARE_ASM(curr_command, "hlt", HLT, opcode);
            BITWISE_COMPARE_ASM(curr_command, "in",  IN, opcode);
            BITWISE_COMPARE_ASM(curr_command, "mul", MUL, opcode);
            BITWISE_COMPARE_ASM(curr_command, "add", ADD, opcode);


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

    POINTERS_DUMP(pointers, pointers_counter);

    buff = ref_buff;
    strcpy(curr_command, "");

    STRIKE_ME_OUT();
    printf("\n");
    STRIKE_ME_OUT();
    printf("\n");


    for(int i = 3; buff[1];){
        BEAUTY_BIN_DUMP("Previous");

        if(sscanf(buff, "%s%n", curr_command, &len) && curr_command[len - 2] != ':' && curr_command[len - 1] != ':'){
            // ???
            // CHECKPOINT("DA YA DAUNICH))");
            i++;
        }


        buff += len;
        if(sscanf(buff, "%d", &int_arg)){
            i += sizeof(int);
        }

        // printf("curr_command: %s, len = %zu, i = %d\n\n", curr_command, len, i);

        if(*buff == ' '){
            if(!strcmp(curr_command, "jb") || !strcmp(curr_command, "jmp")){
                printf("yep.");
                buff++;

                if(sscanf(buff, "%s%n:", curr_arg, &len) <= 0){
                    printf("\033[1;31mError\033[0m: Wrong pointer for jb.\n");
                    return -1;
                }

                printf("\ncurr_arg: %s, len = %zu, i = %d\n", curr_arg, len, i);

                curr_arg[len - 1] = '\0';
                int_arg = simple_pointer_search(pointers, curr_arg, pointers_counter);

                if(int_arg == -1){
                    printf("\033[1;31mError\033[0m: No such pointer was found for jb.\nExiting...\n");
                    return -1;
                }
                WRITE_INT(i, pointers[int_arg].address);
                i += sizeof(int);
            }
            buff++;

            SKIP_STR();
        }
        BEAUTY_BIN_DUMP("After");
        STRIKE_ME_OUT();
        printf("\n\n\n");
    }

    printf("\nchar_binary_code: binary_pos_counter = %zu\n", binary_pos_counter);
    for(size_t i = 0; i < binary_pos_counter; i++){
        printf("%3hhu ", char_binary_code[i]);
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

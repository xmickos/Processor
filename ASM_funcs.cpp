#include "main_header.hpp"
#include "stack_funcs.cpp"

int read_from_file(const char* filename, MyFileStruct* FileStruct, FILE* logfile){
    FILE* input_file = fopen(filename, "r");
    struct stat filestat;

    stat(filename, &filestat);
    FileStruct->filesize = (size_t)filestat.st_size;

    FileStruct->buff = (char*)calloc(FileStruct->filesize + 1, sizeof(char));
    if(FileStruct->buff == nullptr){
        printf("Bad calloc!\n");
        return -1;
    }


    if(fread(FileStruct->buff, sizeof(char), FileStruct->filesize, input_file) < FileStruct->filesize){
        fprintf(logfile, "fread readed less than size!\n");
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
    #ifndef INTEGER_LOGIC
    #ifndef CHAR_LOGIC
    printf("\033[1;32mERROR\033[0m PLEASE DEFINE CHAR_LOGIC OR INTEGER_LOGIC. EXITING...\n");
    exit(-1);
    #endif
    #endif
    const char* buff = FileStruct->buff;
    size_t line_counter = 0, len = 0, binary_pos_counter = 0;
    int int_command = VM_POISON, int_arg = VM_POISON, *binary_code = nullptr;
    bool silent_arg = false;
    char curr_command[INIT_LEN] = {}, curr_arg[INIT_LEN] = {}, output_str[INIT_LEN] = {}, version[2 * INIT_LEN] = {};

    unsigned char final_processing_unit = (char)0u, bit_command = (char)0;
    unsigned char *char_binary_code = nullptr;


    // TODO: think whether to use char's instead of int's

    // imm/reg  reg num   opcode
    //    0        00     00000


    fprintf(logfile, "Initial buff::\n%s\n", buff);

    sprintf(version, "VERSION: %d, %d REGISTERS, %d COMMANDS\n", CPU_VERSION, NUM_OF_REGS, NUM_OF_COMMANDS);
    fputs(version, output);

    binary_code = (int*)calloc(2 * (FileStruct->num_of_str) + 4, sizeof(int)); // + 4 = 1 + cpu_version + num_of_regs +
    char_binary_code = (unsigned char*)calloc(2 * (FileStruct->num_of_str) + 4, sizeof(char)); //  + command_count

    // #ifdef INTEGER_LOGIC
    // binary_code[0] = CPU_VERSION;
    // binary_code[1] = NUM_OF_REGS;
    // binary_code[2] = NUM_OF_COMMANDS;
    // #endif

    // #ifdef CHAR_LOGIC
    char_binary_code[0] = (char)CPU_VERSION;
    char_binary_code[1] = (char)NUM_OF_REGS;
    char_binary_code[2] = (char)NUM_OF_COMMANDS;
    // #endif

    binary_pos_counter += 3;

    while(buff[1]){
        final_processing_unit = (char)0;

        if(!isalpha(*buff)) SKIP_STR();

        if(sscanf(buff, "%4s%n", curr_command, &len) == 0){         // 4 == STRINGIFY(INIT_LEN - 1)
            fprintf(logfile, "Failed to read command!\n");
        }

        buff += len;
        fprintf(logfile, "\ncommand: %s, buff::%s\n", curr_command, buff);

        if(*buff == '\n' || sscanf(buff, "%s", curr_arg) == 0 || *buff == '\0'){
            fprintf(logfile, "Failed to read argument!\n");
            silent_arg = true;
        }

        fprintf(logfile, "curr_command = %s, curr_arg = %s, Silentness = %d\n", curr_command, curr_arg, silent_arg);

        CHECKPOINT("first check");

        // #ifdef INTEGER_LOGIC
        // COMMAND_COMPARE_ASM("push", PUSH, curr_command, int_command);
        // COMMAND_COMPARE_ASM("pop", RPOP, curr_command, int_command);
        // if(int_command != VM_POISON){
        // #endif

        // #ifdef CHAR_LOGIC
        printf("curr command: %s\n", curr_command);
        BITWISE_COMPARE_ASM("push", BPUSH, curr_command, final_processing_unit);
        BITWISE_COMPARE_ASM("pop", BPOP, curr_command, final_processing_unit);
        if((final_processing_unit & COMMAND_BITS) != 0u){
        // #endif

            CHECKPOINT("non-poison - case");

            if(silent_arg == true){
                fprintf(logfile, "Error: Wrong command at %zu line.\n", line_counter + 1);
                return -1;
            }

            if(curr_arg[0] == 'r'){
                fprintf(logfile, "curr arg: %s, register case.\n", curr_arg);
                printf("curr arg: %s, register case.\n", curr_arg);

//                 #ifdef INTEGER_LOGIC
//                 REGISTER_COMPARE_ASM("rax", RAX, curr_arg, int_arg);
//                 REGISTER_COMPARE_ASM("rbx", RBX, curr_arg, int_arg);
//                 REGISTER_COMPARE_ASM("rcx", RCX, curr_arg, int_arg);
//                 REGISTER_COMPARE_ASM("rdx", RDX, curr_arg, int_arg);
//                 if(int_arg == VM_POISON){
//
//                     fprintf(logfile, "Error: Unknown argument at %zu line: %s\n", line_counter, curr_arg);
//                     printf("Error: Unknown argument at %zu line: %s\n", line_counter, curr_arg);
//
//                     return -1;
//                 }
//                 #endif

                // #ifdef CHAR_LOGIC
                BITWISE_COMPARE_ASM("rax", BRAX, curr_arg, final_processing_unit);
                BITWISE_COMPARE_ASM("rbx", BRBX, curr_arg, final_processing_unit);
                BITWISE_COMPARE_ASM("rcx", BRCX, curr_arg, final_processing_unit);
                BITWISE_COMPARE_ASM("rdx", BRDX, curr_arg, final_processing_unit);
                // #endif

                printf("final_processing_unit after regs is %d\n", final_processing_unit);
                fprintf(logfile, "final_processing_unit after regs is %d\n", final_processing_unit);


                final_processing_unit |= REGISTER_BIT;
                char_binary_code[binary_pos_counter++] = final_processing_unit;

                printf("final_processing_unit after regs and is %d\n", final_processing_unit);
                fprintf(logfile, "final_processing_unit after regs and is %d\n", final_processing_unit);

            }else{                                          // 'else' for curr_arg[0] != 'r' case
                int_arg = atoi(curr_arg);
                final_processing_unit &= ~IMREG_BIT;
                char_binary_code[binary_pos_counter++] = final_processing_unit;
                char_binary_code[binary_pos_counter++] = (int_arg & FIRST_INT_BYTE) >> 24; /* (4 bytes <=> 32 bits) */
                char_binary_code[binary_pos_counter++] = (int_arg & SECOND_INT_BYTE) >> 16;
                char_binary_code[binary_pos_counter++] = (int_arg & THIRD_INT_BYTE)  >> 8;
                char_binary_code[binary_pos_counter++] = int_arg & BHLT; /* BHLT == LAST_INT_BYTE*/
            }

            // #ifdef INTEGER_LOGIC
            // sprintf(output_str, "%d %d\n", int_command, int_arg);
            // binary_code[binary_pos_counter++] = int_command;
            // #endif

        }
        else{
            CHECKPOINT("after regs");
            printf("FPU after regs: %d\n", final_processing_unit);
            fprintf(logfile, "FPU after regs: %d\n", final_processing_unit);

//             #ifdef INTEGER_LOGIC
//             COMMAND_COMPARE_ASM("div", DIV, curr_command, int_command);
//             COMMAND_COMPARE_ASM("sub", SUB, curr_command, int_command);
//             COMMAND_COMPARE_ASM("out", OUT, curr_command, int_command);
//             COMMAND_COMPARE_ASM("hlt", HLT, curr_command, int_command);
//             COMMAND_COMPARE_ASM("in",  IN, curr_command, int_command);
//             COMMAND_COMPARE_ASM("mul", MUL, curr_command, int_command);
//
//             sprintf(output_str, "%d\n", int_command);
//             binary_code[binary_pos_counter++] = int_command;
//             binary_code[binary_pos_counter++] = int_arg;
//             #endif

            // #ifdef CHAR_LOGIC
            BITWISE_COMPARE_ASM("div", BDIV, curr_command, final_processing_unit);
            BITWISE_COMPARE_ASM("sub", BSUB, curr_command, final_processing_unit);
            BITWISE_COMPARE_ASM("out", BOUT, curr_command, final_processing_unit);
            BITWISE_COMPARE_ASM("hlt", BHLT, curr_command, final_processing_unit);
            BITWISE_COMPARE_ASM("in",  BIN,  curr_command, final_processing_unit);
            BITWISE_COMPARE_ASM("mul", BMUL, curr_command, final_processing_unit);
            // #endif

            char_binary_code[binary_pos_counter++] = final_processing_unit;

            printf("FPU after all: %d\n", final_processing_unit);
            fprintf(logfile, "FPU after all: %d\n", final_processing_unit);


            CHECKPOINT("one sprintf");
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
        int_command = VM_POISON;
        int_arg = VM_POISON;
    }
    #ifdef INTEGER_LOGIC
    binary_code[binary_pos_counter++] = VM_POISON;
    if(fwrite(binary_code, sizeof(int), binary_pos_counter + 2, bin_output) < binary_pos_counter){
        printf("fwrite failed!\n");
        return -1;
    }
    #endif

    #ifdef CHAR_LOGIC
    if(fwrite(char_binary_code, sizeof(char), binary_pos_counter + 2, bin_output) < binary_pos_counter){
        printf("fwrite failed!\n");
        return -1;
    }
    #endif


    #ifdef DEBUG
    printf("Binary code array:\n");
    for(size_t i = 0; i < binary_pos_counter; i++){
        if(binary_code[i] == VM_POISON){
            printf("\n");
            continue;
        }
        printf("%d ", binary_code[i]);
    }
    #endif
    free(binary_code);
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

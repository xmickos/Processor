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

void string_processing_asm(char* buff, FILE* output, FILE* bin_output, FILE* logfile){
    size_t line_counter = 0, len = 0, binary_pos_counter = 0;
    int int_command = VM_POISON, int_arg = VM_POISON, *binary_code = nullptr;
    bool silent_arg = false;
    char curr_command[INIT_LEN] = {}, curr_arg[INIT_LEN] = {}, output_str[INIT_LEN] = {}, version[2 * INIT_LEN] = {};

    fprintf(logfile, "Initial buff::\n%s\n", buff);

    sprintf(version, "VERSION: %d, %d REGISTERS, %d COMMANDS\n", CPU_VERSION, NUM_OF_REGS, NUM_OF_COMMANDS);
    fputs(version, output);

    binary_code = (int*)calloc(strlen(buff) + 4, sizeof(int)); //3 + 1 = 4forCPU_VERSION NUM_OF_REGS VM_POISON NUM_OF_CO

    binary_code[0] = CPU_VERSION;
    binary_code[1] = NUM_OF_REGS;
    binary_code[2] = NUM_OF_COMMANDS;
    binary_pos_counter+=3;

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
            fprintf(logfile, "Wroten string: %s\n", output_str);
        }

        if(!silent_arg){
            SKIP_STR();
        }
        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;

        line_counter++;
    }

    binary_code[binary_pos_counter++] = VM_POISON;

    fwrite(binary_code, sizeof(int), binary_pos_counter + 2, bin_output);

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
}

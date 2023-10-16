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

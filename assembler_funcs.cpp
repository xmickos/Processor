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

void string_processing_asm(char* buff, FILE* output, FILE* logfile){
    size_t i = 0;
    int curr_arg = -1, int_command = -2;
    bool silent_arg = false;
    char curr_command[5], output_str[25];

    while(*buff){
        if(sscanf(buff, "%s", curr_command) == 0){
            fprintf(logfile, "Failed to read command!\n");
        }

        buff += strlen(curr_command) + 1;
        fprintf(logfile, "\nbuff::%s\n", buff);

        if(sscanf(buff, "%d", &curr_arg) == 0 || *buff == '\0'){
            fprintf(logfile, "Failed to read argument!\n");
            silent_arg = true;
        }

        COMMAND_COMPARE_ASM("push", PUSH);
        COMMAND_COMPARE_ASM("div", DIV);
        COMMAND_COMPARE_ASM("sub", SUB);
        COMMAND_COMPARE_ASM("out", OUT);
        COMMAND_COMPARE_ASM("pop", POP);
        COMMAND_COMPARE_ASM("hlt", HLT);
        COMMAND_COMPARE_ASM("in",  IN);
        COMMAND_COMPARE_ASM("mul", MUL);



        if(silent_arg){
            sprintf(output_str, "%d\n", int_command);
        }
        else{
            sprintf(output_str, "%d %d\n", int_command, curr_arg);
        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully! Silentness: %d\n", silent_arg);
            fprintf(logfile, "Wroten string: %s\n", output_str);
        }

        if(!silent_arg){
            while(buff[i] != '\n') i++;
            buff += i + 1;
            i = 0;
        }
        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;
    }
}

void string_processing_disasm(char* buff, FILE* output, FILE* logfile){
    int int_command = -2;
    bool silent_arg = false;
    char curr_command[5], output_str[25], trash[25], curr_arg[5];

    while(*buff){
        sscanf(buff, "%s", curr_command);
        buff += strlen(curr_command);
        printf("curr command: %s, strlen + 1 = %lu\n", curr_command, strlen(curr_command) + 1);

        switch(*buff){
            case ' ':
                sscanf(buff, "%s", curr_arg);
                buff += strlen(curr_arg);
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

        COMMAND_COMPARE_DISASM("push", PUSH);
        COMMAND_COMPARE_DISASM("div", DIV);
        COMMAND_COMPARE_DISASM("sub", SUB);
        COMMAND_COMPARE_DISASM("out", OUT);
        COMMAND_COMPARE_DISASM("pop", POP);
        COMMAND_COMPARE_DISASM("hlt", HLT);
        COMMAND_COMPARE_DISASM("in" , IN);
        COMMAND_COMPARE_DISASM("mul", MUL);

        printf("\nreaded::%s, %s\n", curr_command, curr_arg);

        if(silent_arg){
            sprintf(output_str, "%s\n", curr_command);
        }
        else{
            sprintf(output_str, "%s %s\n", curr_command, curr_arg);
        }

        if(fputs(output_str, output)){
            fprintf(logfile, "String written successfully! Silentness: %d\n", silent_arg);
            fprintf(logfile, "Wroten string: %s\n", output_str);
        }

        fprintf(logfile, "newbuff::%s\n", buff);
        silent_arg = false;
    }
}

void kernel(const char* buff, Stack *stk, FILE* logfile){
    int int_command = -2;
    bool silent_arg = false;
    Elem_t first_operand = 0, second_operand = 1;
    char curr_command[5], output_str[25], trash[25], curr_arg[5];

    while(*buff){
        sscanf(buff, "%s", curr_command);
        buff += strlen(curr_command);
        // printf("curr command: %s, strlen = %lu\n", curr_command, strlen(curr_command));

        switch(*buff){
            case ' ':
                sscanf(buff, "%s", curr_arg);
                buff += strlen(curr_arg);
                // printf("case 1\n");
                while(*buff != '\n') buff++;
                buff++;
                break;
            case '\n':
                *curr_arg = '\0';
                silent_arg = true;
                buff++;
                // printf("case 2\n");
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
                // printf("Case PUSH\n");
                StackPush(stk, logfile, atoi(curr_arg));
                break;
            case POP:
                // printf("Case POP\n");
                StackPop(stk, logfile, &first_operand);
                printf("%f\n", first_operand);
                break;
            case DIV:
                // printf("Case DIV\n");
                StackPop(stk, logfile, &second_operand);
                // fscanf(stdin, "%lf", &second_operand);
                StackPop(stk, logfile, &first_operand);
                // fscanf(stdin, "%lf", &first_operand);
                if(!IsEqual(second_operand, EPS)){
                    StackPush(stk, logfile, first_operand / second_operand);
                }
                else{
                    printf("Divising by zero!\n");
                }
                break;
            case SUB:
                // printf("Case SUB\n");
                StackPop(stk, logfile, &second_operand);
                // fscanf(stdin, "%lf", &second_operand);
                StackPop(stk, logfile, &first_operand);
                // fscanf(stdin, "%lf", &first_operand);
                StackPush(stk, logfile, first_operand - second_operand);
                break;
            case IN:
                fscanf(stdin, "%lf", &first_operand);
                StackPush(stk, logfile, first_operand);
                break;
            case MUL:
                StackPop(stk, logfile, &first_operand);
                StackPop(stk, logfile, &second_operand);
                StackPush(stk, logfile, first_operand * second_operand);
                break;
            case OUT:
                // printf("Case OUT\n");
                StackPop(stk, logfile, &first_operand);
                // fscanf(stdin, "%lf", &first_operand);
                fprintf(stdout, "\t\t\t\t[OUT]: %f\n", first_operand);
                break;
            case HLT:
                // printf("Case HLT\n");
                fprintf(stdout, "HALT: exiting...\n");
                exit(-1);
        }

    }

}

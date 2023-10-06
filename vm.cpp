#include "main_header.hpp"

int main(){
    FILE *logfile = fopen("vm_logfile.txt", "w");
    FILE *input_file = fopen("assembler_output.txt", "r");
    char name[25] = "assembler_output.txt", *buff = nullptr;
    Stack stk = {};

    StackCtor(&stk, 10, logfile);
    buff = read_from_file(name, logfile);

    printf("\nReaded buff:%s", buff);

    kernel(buff, &stk, logfile);


    free(buff);
    fclose(input_file);
    fclose(logfile);
    return 0;
}

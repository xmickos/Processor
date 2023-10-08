#include "main_header.hpp"

int main(){
    FILE *logfile = fopen("vm_logfile.txt", "w");
    FILE *input_file = fopen("assembler_output.txt", "r");
    char name[25] = "assembler_output.txt", *buff = nullptr;
    Processor cpu = {};

    StackCtor(&cpu.stk, 10, logfile);
    buff = read_from_file(name, logfile);

    // printf("\nReaded buff:\n%s", buff);

    kernel(buff, &cpu, logfile);


    free(buff);
    fclose(input_file);
    fclose(logfile);
    return 0;
}

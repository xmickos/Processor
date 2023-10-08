#include "main_header.hpp"

int main(){
    FILE *logfile = fopen("vm_logfile.txt", "w");
    FILE *input_file = fopen("assembler_output.txt", "r");
    char name[25] = "assembler_output.txt", *buff = nullptr;
    Processor cpu = {};

    if(CpuCtor(&cpu, 10, logfile) < 0){
        fprintf(logfile, "Failed to create cpu!\n");
        return 0;
    }

    buff = read_from_file(name, logfile);

    // printf("\nReaded buff:\n%s", buff);

    kernel(buff, &cpu, logfile);

    CpuDtor(&cpu, logfile);


    free(buff);
    fclose(input_file);
    fclose(logfile);
    return 0;
}

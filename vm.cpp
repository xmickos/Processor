#include "main_header.hpp"

int main(){
    FILE *logfile = fopen("vm_logfile.txt", "w");
    FILE *input_file = fopen("assembler_output.txt", "r");
    FILE *bin_input = fopen("bin_assembler_output.bin", "rb");
    // char name[25] = "assembler_output.txt";
    char bin_name[25] = "bin_assembler_output.bin", *buff = nullptr;
    int *bin_buff = nullptr;
    Processor cpu = {};

    if(CpuCtor(&cpu, 10, logfile) < 0){
        fprintf(logfile, "Failed to create cpu!\n");
        return 0;
    }

    // buff = read_from_file(name, logfile);

    bin_buff = read_from_bin_file(bin_name, logfile);

    printf("\nReaded bin buff: \n");
    for(int i = 0; bin_buff[i] != VM_POISON; i++){
        printf("%d\n", bin_buff[i]);
    }

    kernel(nullptr, &cpu, logfile, bin_buff);

    CpuDtor(&cpu, logfile);

    // printf("\033[1;32mWELCOME TO DOLGOPA\033[0m\n");


    free(buff);
    fclose(bin_input);
    fclose(input_file);
    fclose(logfile);
    return 0;
}

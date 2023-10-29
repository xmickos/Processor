#include "main_header.hpp"

int main(const int argc, const char **argv){

    if(argc < 2){
        printf("No file was provided!\n");
        return 0;
    }

    if(argv[1] == nullptr){
        printf("Bad commandline argument.\n");
        return 0;
    }

    printf("ARGV: %s\n", argv[1]);
    FILE *logfile = fopen("vm_logfile.txt", "w");
    Processor cpu = {};


    if(CpuCtor(&cpu, CPU_INIT_CAP, logfile) < 0){
        fprintf(logfile, "Failed to create cpu!\n");
        return 0;
    }

    unsigned char* bin_buff = nullptr;
    const char *bin_name = argv[1];

    bin_buff = read_from_bin_file(bin_name, logfile);

    printf("\nRead bin buff: \n");
    for(int i = 1; bin_buff[i - 1] != COMMAND_BITS; i++){
        printf("%d\n", bin_buff[i]);
    }

    kernel(&cpu, logfile, bin_buff);

    CpuDtor(&cpu, logfile);

    // printf("\033[1;32mWELCOME TO DOLGOPA\033[0m\n");

    fclose(logfile);
    return 0;
}

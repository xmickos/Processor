#include "main_header.hpp"

int main(const int argc, const char **argv){
    printf("ARGV: %s\n", argv[1]);
    FILE *logfile = fopen("vm_logfile.txt", "w");
    Processor cpu = {};

    if(CpuCtor(&cpu, CPU_INIT_CAP, logfile) < 0){
        fprintf(logfile, "Failed to create cpu!\n");
        return 0;
    }

    #ifdef NO_BINARY_READ
    const char *name = argv[1];
    buff = read_from_file(name, logfile);

    kernel(buff, &cpu, logfile, nullptr);

    free(buff);
    #endif

    #ifndef NO_BINARY_READ
    int *bin_buff = nullptr;
    const char *bin_name = argv[1];

    bin_buff = read_from_bin_file(bin_name, logfile);

    printf("\nRead bin buff: \n");
    for(int i = 0; bin_buff[i] != VM_POISON; i++){
        printf("%d\n", bin_buff[i]);
    }

    kernel(nullptr, &cpu, logfile, bin_buff);
    #endif

    CpuDtor(&cpu, logfile);

    // printf("\033[1;32mWELCOME TO DOLGOPA\033[0m\n");

    fclose(logfile);
    return 0;
}

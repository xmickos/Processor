#include "main_header.hpp"

int main(const int argc, const char** argv){
    if(argv[1] == nullptr){
        printf("Bad commandline argument.\n");
        return 0;
    }

    FILE *logfile = fopen("assembler_logfile.txt", "w");
    FILE *output = fopen("assembler_output.bin", "wb");
    FILE *bin_output = fopen("bin_assembler_output.bin", "wb");
    const char *name = argv[1];
    char *buff = nullptr;

    buff = read_from_file(name, logfile);

    printf("Readed buff:\n%s", buff);

    string_processing_asm(buff, output, bin_output, logfile);


    free(buff);
    fclose(bin_output);
    fclose(logfile);
    fclose(output);
    return 0;
}

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
    MyFileStruct FileStruct = {};

    read_from_file(name, &FileStruct, logfile);

    printf("Readed buff:\n%s", FileStruct.buff);

    string_processing_asm(&FileStruct, output, bin_output, logfile);

    // int a = 4;
    // printf("initial num: a = %d, first 8 bites: %d, second 8 bites: %d, third 8 bites: %d, last 8 bites: %b, size: %lu\n",
    // a, (a & 0b11111111000000000000000000000000) >> 24, a & SECOND_INT_BYTE >> 16, a & THIRD_INT_BYTE >> 8, a & BHLT, sizeof(int));


    free(FileStruct.buff);
    fclose(bin_output);
    fclose(logfile);
    fclose(output);
    return 0;
}

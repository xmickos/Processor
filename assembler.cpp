#include "main_header.hpp"

int main(const int argc, const char** argv){
    if(argc == 0){
        printf("No file was provided!\n");
        return 0;
    }
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

    free(FileStruct.buff);
    fclose(bin_output);
    fclose(logfile);
    fclose(output);
    return 0;
}

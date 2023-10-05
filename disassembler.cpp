#include "main_header.hpp"

int main(){
    FILE *input_file = fopen("assembler_output.txt", "r");
    FILE *output_file = fopen("disassembler_output.txt", "w");
    FILE *logfile = fopen("disassembler_logfile.txt", "w");
    char name[21] = "assembler_output.txt";
    char *buff = nullptr;

    buff = read_from_file(name, logfile);

    string_processing_disasm(buff, output_file, logfile);

    printf("buff:\n%s\n", buff);

    fclose(input_file);
    fclose(output_file);
    fclose(logfile);
    free(buff);
    return 0;
}

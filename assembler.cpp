#include "main_header.hpp"

int main(){
    FILE *logfile = fopen("assembler_logfile.txt", "w");
    FILE *output = fopen("assembler_output.txt", "w");
    char name[25] = "user_input.txt";
    char *buff = nullptr;

    buff = read_from_file(name, logfile);


    string_processing_asm(buff, output, logfile);



    free(buff);
    fclose(logfile);
    fclose(output);
    return 0;
}

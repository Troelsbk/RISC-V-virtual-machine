# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <stdbool.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>

# define DEBUG(...) do{if(debug){fprintf(stderr, __VA_ARGS__);}}while(0);



bool is_binary(char *fileName);
char ** get_bin_files(int *size);
int debug = 1;

void get_res_file(char *bin_file){
    char * strrchr_out = strrchr(bin_file, '.');
    memcpy(strrchr_out, ".res", 4);
}


bool compare_bin_files(char *input_file){
    //compares the result from the task with the output generated from the vm
    bool result = true; 
    size_t file_size = 32 * 4; //assumes this file size

    //file handling and check
    FILE * fp_input_file = fopen(input_file, "rb");
    if(fp_input_file == NULL){
        perror("[compare_bin_files]input_file open error");
        exit(1);
    }

    FILE * fp_vm_out = fopen("vm_out.res", "rb");
    if(fp_vm_out == NULL){
        perror("[compare_bin_files]reg_out.bin open error");
        fclose(fp_input_file);
        exit(1);
    }

    uint8_t buffer1[file_size];
    uint8_t buffer2[file_size];
    if((fread(buffer1, sizeof(uint8_t), file_size, fp_input_file)) != file_size){
        perror("[compare_bin_files]:file read erorr to buffer1");;
        fclose(fp_input_file);
        fclose(fp_vm_out);
        exit(1);
    }

    if((fread(buffer2, sizeof(uint8_t), file_size, fp_vm_out)) != file_size){
        perror("[compare_bin_files]:file read erorr to buffer2");;
        fclose(fp_input_file);
        fclose(fp_vm_out);
        exit(1);
    }

    //check each byte
    result = (memcmp(buffer1, buffer2, file_size) == 0);

    fclose(fp_input_file);
    fclose(fp_vm_out);
return result;
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("%s <risv_v_vm file name>\n", argv[0]);
        exit(1);
    }
    int size = 0;
    char ** bin_files = get_bin_files(&size);
    char * current_file;
    int max_size = 40;
    char run_vm_command[max_size];


    for(int index = 0; index < size; index++){
        memset(run_vm_command, 0x00, sizeof(run_vm_command));

        current_file = bin_files[index];
        snprintf(run_vm_command, sizeof(run_vm_command), "./%s %s", argv[1], current_file);
        system(run_vm_command);

        //printf("before get_res_file: %s\n", current_file);
        get_res_file(current_file);
        bool result = compare_bin_files(current_file);
        printf("%-20s %s\n", current_file, result ? "passed": "failed");
    }

    return 0;

}

bool is_binary(char *file_name){
    // check if file_name is binary by the *.bin suffix
    if(file_name == NULL){
        perror("File error");
        exit(1);
    }
    const char * temp1 = strrchr(file_name, '.'); //"real" file name

    if(temp1 != NULL && (strcmp(temp1, ".bin") == 0)){
        return true;
    }

return false;
}

char ** get_bin_files(int *size){
    //read all bin files in current directory

    int bin_files_max = 5;
    int bin_files_index = 0;
    char **bin_files = calloc(bin_files_max, sizeof(char *));
    if(bin_files == NULL){
        perror("Calloc error");
        exit(1);
    }

    DIR * dir = opendir(".");
    if(dir == NULL){
        perror("Dir error.");
        exit(1);
    }

    struct dirent *entry;

    while((entry = readdir(dir)) != NULL){

        if(is_binary(entry->d_name)){
            bin_files[bin_files_index++] = entry->d_name;

            if(bin_files_index == bin_files_max){
                bin_files_max += 5;
                bin_files = realloc(bin_files, bin_files_max * sizeof(char *));
                if(bin_files == NULL){
                    perror("Realloc error");
                    exit(1);
                }
            }
        }
    }
*size = bin_files_index;
return bin_files;
}

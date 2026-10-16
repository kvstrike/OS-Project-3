#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256

int main(int argc, char *argv[]){
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *addr_file = fopen(argv[1], "r"); // Open the file for reading
    if(!addr_file){ 
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    FILE *backing_store = fopen("BACKING_STORE.bin", "rb"); //open the file for reading in binary mode
    if(!backing_store){
        fprintf(stderr, "Error: Could not open backing store\n");
        fclose(addr_file);
        return 1;
    }

    FILE *out1 = fopen("out1.txt", "w"); 
    FILE *out2 = fopen("out2.txt", "w"); 
    FILE *out3 = fopen("out3.txt", "w"); 
    if(!out1 || !out2 || !out3){ // Check if any of the output files could not be opened/created
        fprintf(stderr, "Error: Could not open one or more output files\n");
        fclose(addr_file);
        fclose(backing_store);
        return 1;
    }

    int logical_address;
    while(fscanf(addr_file, "%d", &logical_address) == 1){
        // Process the logical address here
    }

    fclose(addr_file);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}
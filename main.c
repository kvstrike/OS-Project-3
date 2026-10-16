#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256 

int pageTable[NUM_PAGES];

int main(int argc, char *argv[]){
    //implement mem something like char mem[nframe][pagesize]
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
        if(out1 != NULL){
            fclose(out1);
        }
        if(out2 != NULL){
            fclose(out2);
        }
        if(out3 != NULL){
            fclose(out3);
        }
        return 1;
    }

    //initialize page table
    for(int i = 0; i < NUM_PAGES; i++){
        pageTable[i] = -1; // -1 indicates that the page is not in memory
    }

    int logical_address;
    while(fscanf(addr_file, "%d", &logical_address) == 1){
        // Process the logical address here
        logical_address = logical_address & 0xFFFF; // Ensure the logical address is 16 bits

        printf("Logical Address: %d\n", logical_address);
    }

    fclose(addr_file);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}
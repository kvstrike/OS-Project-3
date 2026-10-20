#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256 

int pageTable[NUM_PAGES];

//each row of the 2D array represents a page in physical memory, and each column represents a byte within that page
signed char physicalMemory[NUM_PAGES][PAGE_SIZE]; // 2D array to represent physical memory

int main(int argc, char *argv[]){
    int nextFrame = 0;
    int pageFaults = 0;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *addr_file = fopen(argv[1], "r"); 
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
    if(!out1 || !out2 || !out3){ 
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

        logical_address = logical_address & 0xFFFF; // Ensure the logical address is 16 bits

        int page_number = logical_address/PAGE_SIZE;
        int offset = logical_address % PAGE_SIZE;

        if(pageTable[page_number] == -1){ // Page fault, page is not in memory
            pageFaults++; 
            
            if(nextFrame >= NUM_PAGES){
                fprintf(stderr, "Error: Physical memory is full, cannot load more pages\n");
                fclose(addr_file);
                fclose(backing_store);
                fclose(out1);
                fclose(out2);
                fclose(out3);
                return 1;
            }

            //calculate the starting byte of the page in the backing store
            long backing_store_position = page_number * PAGE_SIZE;

            if(fseek(backing_store, backing_store_position, SEEK_SET) != 0){
                fprintf(stderr, "Error: Could not seek to position %ld in backing store\n", backing_store_position);
                fclose(addr_file);
                fclose(backing_store);
                fclose(out1);
                fclose(out2);
                fclose(out3);
                return 1;
            }

            size_t bytesRead = fread(physicalMemory[nextFrame], sizeof(signed char), PAGE_SIZE, backing_store);
            if(bytesRead != PAGE_SIZE){
                fprintf(stderr, "Error: Could not read full page from backing store\n");
                fclose(addr_file);
                fclose(backing_store);
                fclose(out1);
                fclose(out2);
                fclose(out3);
                return 1;
            }

            pageTable[page_number] = nextFrame; // Update the page table to indicate the page is now in memory

            nextFrame++; // Increment the next frame index
        }
        int frame_number = pageTable[page_number];
        int physical_address = frame_number * PAGE_SIZE + offset;
        signed char value = physicalMemory[frame_number][offset];
        fprintf(out1, "%d\n", logical_address);
        fprintf(out2, "%d\n", physical_address);
        fprintf(out3, "%d\n", value);
    }

    printf("Page Faults: %d\n", pageFaults);

    fclose(addr_file);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}
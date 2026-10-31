#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define NUM_FRAMES 256 // we will change this to try different number of frames
#define TLB_SIZE 16

typedef struct {
    int pageNumber;
    int frameNumber;
} TLBEntry;

int pageTable[NUM_PAGES]; // stores the frame number for each page
int frameTable[NUM_FRAMES]; // stores the page number currently in each frame
TLBEntry tlb[TLB_SIZE];

//each row of the 2D array represents a page in physical memory, and each column represents a byte within that page
char physicalMemory[NUM_FRAMES][PAGE_SIZE]; // 2D array to represent physical memory

int main(int argc, char *argv[]){
    int nextFrame = 0;
    int pageFaults = 0;
    int tlbHits = 0;
    
    int lastUsedFrame[NUM_FRAMES]; // Array to track the last used time for each frame
    int tlbLastUsed[TLB_SIZE]; // Array to track the last used time for each TLB entry
    int accessCounter = 0; // Global access counter to track the order of accesses

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

    // initialize frame table
    for(int i = 0; i < NUM_FRAMES; i++){
        frameTable[i] = -1; // -1 indicates that the frame is empty
        lastUsedFrame[i] = -1; // Initialize last used time for each frame
    }

    for(int i = 0; i < TLB_SIZE; i++){
        tlb[i].pageNumber = -1; // -1 indicates that the TLB entry is empty
        tlb[i].frameNumber = -1;
        tlbLastUsed[i] = -1; // Initialize last used time for each TLB entry
    }

    int logical_address;

    while(fscanf(addr_file, "%d", &logical_address) == 1){

        logical_address = logical_address & 0xFFFF; // Ensure the logical address is 16 bits

        int page_number = logical_address/PAGE_SIZE;
        int offset = logical_address % PAGE_SIZE;

        //search the TLB for the page number
        int frame_number = -1;
        int tlbEntry = -1;
        for(int i = 0; i < TLB_SIZE; i++){
            if(tlb[i].pageNumber == page_number){
                frame_number = tlb[i].frameNumber;
                tlbEntry = i; // Store the index of the TLB entry
                tlbHits++;
                break;
            }
        }

        // If the page number is not found in the TLB, check the page table
        if(frame_number == -1){
            if(pageTable[page_number] == -1){ // Page fault, page is not in memory
                pageFaults++;

                if(nextFrame < NUM_FRAMES){
                    // use the next available frame
                    frame_number = nextFrame;
                    nextFrame++;
                } 
                else{
                    // Memory full so use LRU page replacement
                    int lruFrame = 0;
                    for(int i = 1; i < NUM_FRAMES; i++){
                        if(lastUsedFrame[i] < lastUsedFrame[lruFrame]){
                            lruFrame = i;
                        }
                    }
                    frame_number = lruFrame;

                    // find the page number that is currently in the victim frame
                    int oldPage = frameTable[frame_number]; 
                    
                    // Remove the old page from the page table and TLB
                    pageTable[oldPage] = -1;
                    for(int i = 0; i < TLB_SIZE; i++){
                        if(tlb[i].pageNumber == oldPage){
                            tlb[i].pageNumber = -1; // Invalidate the TLB entry
                            tlb[i].frameNumber = -1;
                            tlbLastUsed[i] = -1; // Invalidate the last used time for this TLB entry
                            break;
                        }
                    }
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

                size_t bytesRead = fread(physicalMemory[frame_number], sizeof(signed char), PAGE_SIZE, backing_store);
                if(bytesRead != PAGE_SIZE){
                    fprintf(stderr, "Error: Could not read full page from backing store\n");
                    fclose(addr_file);
                    fclose(backing_store);
                    fclose(out1);
                    fclose(out2);
                    fclose(out3);
                    return 1;
                }

                pageTable[page_number] = frame_number; // Update the page table to indicate the page is now in memory and which frame it is in
                frameTable[frame_number] = page_number; // Update the frame table to indicate which page is in this frame
            }

            frame_number = pageTable[page_number];
            for(int i = 0; i < TLB_SIZE; i++){
                if(tlb[i].pageNumber == -1){ 
                    tlbEntry = i;
                    break;
                }
            }

            if(tlbEntry == -1){ // If TLB is full, use LRU replacement
                tlbEntry = 0;
                for(int i = 1; i < TLB_SIZE; i++){
                    if(tlbLastUsed[i] < tlbLastUsed[tlbEntry]){
                        tlbEntry = i;
                    }
                }
            }
            
            tlb[tlbEntry].pageNumber = page_number;
            tlb[tlbEntry].frameNumber = frame_number;
        }

        tlbLastUsed[tlbEntry] = accessCounter; // Update the last used time for this TLB entry
        lastUsedFrame[frame_number] = accessCounter; // Update the last used time for this frame
        accessCounter++; // Increment the global access counter
        int physical_address = frame_number * PAGE_SIZE + offset;
        signed char value = physicalMemory[frame_number][offset];
        fprintf(out1, "%d\n", logical_address);
        fprintf(out2, "%d\n", physical_address);
        fprintf(out3, "%d\n", value);
    }

    printf("Page Faults: %d\n", pageFaults);
    printf("TLB Hits: %d\n", tlbHits);

    fclose(addr_file);
    fclose(backing_store);
    fclose(out1);
    fclose(out2);
    fclose(out3);
    return 0;
}
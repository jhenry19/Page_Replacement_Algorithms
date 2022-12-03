#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>

#define BUFFLEN 1024
#define FILENAME "../small-loop.atrace.out"

enum ReplacementPolicy { FIFO, LRU, RAND };

typedef struct {
    int inUse; // whether there is actually an active page in this frame
    int inTime; // the time that the page is this frame was loaded
    int useTime; // the most recent access time for the page in this frame
    unsigned long pageNum; // the page in this frame
} PageTableEntry;

typedef struct {
    int numFrames; // number of page frames available
    unsigned int currentTime; // a counter; increment it each time translate() is called
    enum ReplacementPolicy policy; // an enum representing which replacement alg to use
    int numPageFaults; // the total number of page faults
    int numReferences; // the total number of references to the page table
} MemStruct;

/**
 * Determines a free frame to be used depending on the replacement policy.
 * @param pageTable
 * @param pageNum
 * @param iFrameNum the frame number of the instructions if it is a data page
 * @param memStruct the memory struct for this run includs what page replacement algorithm to use
 * @return
 */
unsigned long getFreeFrame(PageTableEntry * pageTable, unsigned long pageNum, unsigned long iFrameNum, MemStruct *memStruct) {
    // Declare variables to which page to fill for FIFO or LRU replacement policy
    int earliestInTime = INT_MAX;
    int FIFOIndex = -1;
    int earliestUseTime = INT_MAX;
    int LRUIndex = -1;

    PageTableEntry currentPage;
    // Loop through page table
    for (int i = 0; i < memStruct->numFrames; ++i) {
        currentPage = pageTable[i];
        if (currentPage.inUse == 0) { // if there is an unused page, put the page there
            printf("%s %d\n", "found unused target frame: ", i);
            return i;
        }
        else if (i != iFrameNum) { // Only checks frames that are not the instructions for the data
            // FIFO
            if (currentPage.inTime < earliestInTime) { // if currentPage is the earliest
                earliestInTime = currentPage.inTime;
                FIFOIndex = i;
            }
            // LRU
            if (currentPage.useTime < earliestUseTime) {
                earliestUseTime = currentPage.useTime;
                LRUIndex = i;
            }
        }
    }

    //If no unused page, choose page based on replacement policy
    if (memStruct->policy == FIFO) { // FIFO
        printf("evict page in frame %d\n", FIFOIndex);
        return FIFOIndex;
    }
    else if (memStruct->policy == LRU) { // LRU

        return LRUIndex;
    }
    else { // random
        int randomPage;
        int searchingForPage = 1;
        while(searchingForPage == 1) { // while loop so that instruction page is not evicted
            randomPage = rand() % memStruct->numFrames;
            if (randomPage != iFrameNum) {
                printf("evict page in frame %d\n", randomPage);
                searchingForPage = 0;
                return randomPage;
            }
        }
    }
}

/**
 * Translates the virtual address into a physical address in the page table.
 * @param pageTable the page table itself
 * @param pageNum the virtual page being translated
 * @param iFrameNum the frame number of the instruction address corresponding to this data page; or zero if pageNum corresponds to an instruction address
 * @param memStruct information about the virtual memory
 */
unsigned long translate(PageTableEntry *pageTable, unsigned long pageNum, unsigned long iFrameNum, MemStruct *memStruct) {
    ++memStruct->numReferences; // increment number of references

    // Look at all entries in page table
    PageTableEntry* currentPage;
    for (int i = 0; i < memStruct->numFrames; ++i) {
        currentPage = &pageTable[i];
        if (currentPage->pageNum == pageNum && currentPage->inUse != 0) { // Check if page already exists
            printf("page %lu : already in frame %d\n", pageNum, i);
            currentPage->inUse = memStruct->currentTime;
            return i;
        }
    }

    // If page is not in the table, this is a page fault, increment numPageFaults counter
    ++memStruct->numPageFaults;
    unsigned int freeFrameIndex = getFreeFrame(pageTable, pageNum, iFrameNum, memStruct);

    // Creates page to be added
    PageTableEntry toAdd;
    toAdd.inUse = 1;
    toAdd.pageNum = pageNum;
    toAdd.inTime = memStruct->currentTime;
    toAdd.useTime = memStruct->currentTime;
    pageTable[freeFrameIndex] = toAdd;
    ++memStruct->currentTime;

    printf("fault:page %lu -> frame %d\n", pageNum, freeFrameIndex);

    return freeFrameIndex;
}


// From sscanf-example.c
int readBuffer(PageTableEntry pageTable[], MemStruct *memStruct) {
    char buffer[BUFFLEN];
    char tmpbuf[64];
    unsigned long val1, val2;
    int nf;
    FILE *fp;
    char *chp;

    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        printf("cannot open file '%s' for reading\n", FILENAME);
        return 8;
    }

    unsigned long iPageNum;
    unsigned long dPageNum;
    int iFrameNum;
    int dFrameNum;
    chp = fgets(buffer, BUFFLEN, fp);
    while ( chp != NULL) {
        nf = sscanf(buffer, "%lx: %c %lx", &val1, tmpbuf, &val2);
        if (nf == 3) {
            // Calculate iPageNum and dPageNum
            iPageNum = val1 / 4096;
            dPageNum = val2 / 4096;
            iFrameNum = translate(pageTable, iPageNum, -1, memStruct);
            dFrameNum = translate(pageTable, dPageNum, iFrameNum, memStruct);

            printf("%lu -> %d | %lu -> %d\n", iPageNum, iFrameNum, dPageNum, dFrameNum);
        }
        chp = fgets(buffer, BUFFLEN, fp);
    }
    fclose(fp);

    return 0;
}


int main() {
    srand(time(NULL)); // For random

    // Create memory structure
    MemStruct pagingAlgorithm;
    pagingAlgorithm.numFrames = 32;
    pagingAlgorithm.currentTime = 0;
    pagingAlgorithm.policy = RAND;
    pagingAlgorithm.numPageFaults = 0;
    pagingAlgorithm.numReferences = 0;

    // Create page table and populate with empty pages
    PageTableEntry pageTable[pagingAlgorithm.numFrames];
    for (int i = 0; i < pagingAlgorithm.numFrames; ++i) {
        PageTableEntry page;
        page.inUse = 0;
        page.pageNum = -1;
        pageTable[i] = page;
    }

    // Print memory structure info
    printf("policy: %u - %d\n", pagingAlgorithm.policy);
    printf("#frames = %d\n", pagingAlgorithm.numFrames);
    printf("filename: %s\n", FILENAME);

    // Reads file and populates page table
    readBuffer(pageTable, &pagingAlgorithm);

    // Print information about run
    printf("\n#refs  = %d\n", pagingAlgorithm.numReferences);
    printf("#faults  = %d\n\n", pagingAlgorithm.numPageFaults);

    // Print the ending state of the page table
    for (int i = 0; i < pagingAlgorithm.numFrames; ++i) {
        printf("%lu -> %d\n", pageTable[i].pageNum, i);
    }

    return 0;
}

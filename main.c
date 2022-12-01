#include <stdio.h>

#define BUFFLEN 1024
#define FILENAME "../testOne.atrace.out"

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
} MemStruct;

unsigned long translate(
        PageTableEntry * pageTable, // the page table itself
        unsigned long pageNum, // the virtual page being translated
        unsigned long iPageNum, // the page of the instruction address corresponding to this data
        // page; or zero if pageNum corresponds to an instruction address
        MemStruct *memStruct // information about the virtual memory
) {
    //look at entries in page table and
    PageTableEntry currentPage;
    int matchingPathIndex = -1;
    for (int i = 0; i < memStruct->numFrames; ++i) {
        currentPage = pageTable[i];
        if (currentPage.pageNum == pageNum) matchingPathIndex = i;
    }
        // if there is any entry with a matching page number of the address being translated, and for which the inUse
        // flag is not zero, then there is no page fault
        if (currentPage.pageNum == pageNum && currentPage.inUse != 0) {
            printf("%s", "found matching page number\n");
            return i;
        }


        // Notes any unused pages and their index
        else if (indexOfFreeFrame == -1 && currentPage.inUse == 0) {
            printf("%s", "found free page\n");
            indexOfFreeFrame = i; // stops looking for more unused pages once one is found
        }
    }
    //if page is not in the table, this is a page fault, increment numPageFaults counter
    ++memStruct->numPageFaults;

    //if there is an entry not in use then use that frame. set UseTime = currentTime increment the time counter and return the frame number
    if (indexOfFreeFrame != -1) {
        pageTable[indexOfFreeFrame].useTime = memStruct->currentTime;
        pageTable[indexOfFreeFrame].inUse = 1;
        ++memStruct->currentTime;
        printf("%s", "inserted into free page\n");

        return  indexOfFreeFrame;
    }


    unsigned long toReturn = 5;
    return toReturn;
}


// From sscanf-example.c
int readBuffer(PageTableEntry *pageTable, MemStruct *memStruct) {
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
    chp = fgets(buffer, BUFFLEN, fp);
    while ( chp != NULL) {
        nf = sscanf(buffer, "%lx: %c %lx", &val1, tmpbuf, &val2);
        if (nf == 3) {
            //calculate iPageNum and dPageNum
            iPageNum = val1 / 4096;
            dPageNum = val2 / 4096;
            translate(pageTable, val1, iPageNum, memStruct);
            translate(pageTable, val2, dPageNum, memStruct);

            // PageTableEntry * pageTable, // the page table itself
            //        unsigned long pageNum, // the virtual page being translated
            //        unsigned long iPageNum, // the page of the instruction address corresponding to this data
            //        // page; or zero if pageNum corresponds to an instruction address
            //        MemStruct *memStruct // information about the virtual memory
            printf("%s", buffer);
            printf("val1 = %lu; val2 = %lu\n", iPageNum, dPageNum);
        }
        chp = fgets(buffer, BUFFLEN, fp);
    }

    fclose(fp);

    return 0;
}



int main() {
    MemStruct random;
    random.numFrames = 6;
    random.currentTime = 0;
    random.policy = RAND;
    random.numPageFaults = 0;

    PageTableEntry pageTable[random.numFrames];
    readBuffer(pageTable, &random);

    return 0;
}

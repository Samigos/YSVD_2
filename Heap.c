#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BF.h"
#include "Heap.h"

// ------------------------------------------------
// Create a new heap file, open it,
// allocate its first block and write
// to it the number 216, which will work
// as an ID, for heap files. After that,
// close the file.

int HP_CreateFile(char* fileName) {
    void *block;
    int fileDesc, blockNumber = HEAP_FILE_ID;

    if (BF_CreateFile(fileName) < 0) {
        BF_PrintError("Error creating file in HP_CreateFile");
        return -1;
    }
    
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
        BF_PrintError("Error opening file in HP_CreateFile");
        return -1;
    }
    
    // ------------------------------------------------
    
    if (BF_AllocateBlock(fileDesc) < 0) {
        BF_PrintError("Error allocating block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
        BF_PrintError("Error getting block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    // ------------------------------------------------
    
    memcpy(block, &blockNumber, sizeof(int));
    
    if (BF_WriteBlock(fileDesc, 0) < 0) {
        BF_PrintError("Error writing to block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    if (BF_CloseFile(fileDesc) < 0) {
        BF_PrintError("Error closing file in HP_CreateFile");
        return -1;
    }
    
    return 0;
}

// ------------------------------------------------
// Open the heap file with name fileName,
// get its first block, read its content
// and check if the read value, equals 216.

int HP_OpenFile(char* fileName) {
    void *block;
    int fileDesc, blockNumber;
    
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
        BF_PrintError("Error opening file in HP_OpenFile");
        return -1;
    }
    
    if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
        BF_PrintError("Error getting block in HP_OpenFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    // ------------------------------------------------
    
    memcpy(&blockNumber, block, sizeof(int));
    
    if (blockNumber != HEAP_FILE_ID) {
        printf("\n\nError: The file you opened, isn't a heap file!\n\n");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
        
    return fileDesc;
}

// ------------------------------------------------

int HP_CloseFile(int fileDesc) {
    if (BF_CloseFile(fileDesc) == 0) {
        return 0;
    }
    
    BF_PrintError("Error closing file in main");

    return -1;
}

// ------------------------------------------------
// At first, check if the current number of blocks,
// equals 1. If it does, create a new block,
// because we don't need the first one for storing
// entries. After that, update the blockNumber variable,
// that holds the current amount of blocks. Then,
// increase the static int recordsCounter, since a
// new entry is about to be stored in heap file.
// If recordsCounter, exceeds the maximum number of
// entries the current block can store, make the
// recordsCounter to equal 1 and create a new block.
// In the end, read the last created block and
// calculate the position, that the new entry will
// get in this block.

int HP_InsertEntry(int fileDesc, Record record) {
    void* block;
    int numberOfBlocks, numberOfRecordsInBlock = 0;
    
    if ((numberOfBlocks = BF_GetBlockCounter(fileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_InsertEntry");
        return -1;
    }
    else if (numberOfBlocks == 1) {
        if (BF_AllocateBlock(fileDesc) < 0) {
            BF_PrintError("Error allocating block in HP_InsertEntry (1)");
            BF_CloseFile(fileDesc);
            
            return -1;
        }
        
        numberOfBlocks = 2;
    }
    
    // -------------------------------------
    
    const int currentNumberOfBlocks = numberOfBlocks;
    
    if (BF_ReadBlock(fileDesc, numberOfBlocks - 1, &block) < 0) {
        BF_PrintError("Error getting block in HP_InsertEntry");
        return -1;
    }
    
    memcpy(&numberOfRecordsInBlock, block, sizeof(int));
    
    // -------------------------------------
    
    if (++numberOfRecordsInBlock > (int)(BLOCK_SIZE / sizeof(record))) {
        numberOfRecordsInBlock = 1;
        
        if (BF_AllocateBlock(fileDesc) < 0) {
            BF_PrintError("Error allocating block in HP_InsertEntry (2)");
            BF_CloseFile(fileDesc);
            
            return -1;
        }
        
        ++numberOfBlocks;
    }
    
    // -------------------------------------
    
    if (currentNumberOfBlocks != numberOfBlocks && BF_ReadBlock(fileDesc, numberOfBlocks - 1, &block) < 0) {
        BF_PrintError("Error getting block in HP_InsertEntry");
        return -1;
    }
    
    memcpy(block, &numberOfRecordsInBlock, sizeof(int));
    memcpy(block + sizeof(int) + (numberOfRecordsInBlock * sizeof(record)), &record, sizeof(record));
    
    if (BF_WriteBlock(fileDesc, numberOfBlocks - 1) < 0) {
        BF_PrintError("Error writing to block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
	return 0;
}

int HP_SplitFiles(char* initialHeapFileName, const int fieldNo) {
    int numberOfBlocks, blockIndex;
    int initialHeapFileDesc;
    
    if ((initialHeapFileDesc = HP_OpenFile(initialHeapFileName) < 0)) {
        BF_PrintError("Error getting block in HP_SplitFiles");
        return -1;
    }
    
    if ((numberOfBlocks = BF_GetBlockCounter(initialHeapFileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_SplitFiles");
        return -1;
    }
    
    // -------------------------------------
    
    for (blockIndex = 1; blockIndex < numberOfBlocks; blockIndex++) {
        void *block;
        int currentFileDesc;
        
        if (BF_ReadBlock(initialHeapFileDesc, blockIndex, &block) < 0) {
            BF_PrintError("Error getting block in HP_SplitFiles");
            return -1;
        }
        
        int numberOfRecordsInBlock;
        memcpy(&numberOfRecordsInBlock, block, sizeof(int));
        
        int recordIndex;
        Record* records = malloc(sizeof(Record) * numberOfRecordsInBlock);
        
        for (recordIndex = 1; recordIndex <= numberOfRecordsInBlock; recordIndex++) {
            memcpy(&records[recordIndex-1], block + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));
        }
        
        records = bubbleSortedRecords(records, numberOfRecordsInBlock, fieldNo);
        
        // -------------------------------------
        
        char tempFileName[15];
        strcpy(tempFileName, "temp_");
        
        char num[7];
        sprintf(num, "%d", blockIndex);
        strcat(tempFileName, num);
        
        // -------------------------------------
        
        printf("Creating %s heap file...\n", tempFileName);
        
        if (HP_CreateFile(tempFileName) < 0) {
            BF_PrintError("Error creating heap file in HP_SplitFiles");
            return -1;
        }
        
        if ((currentFileDesc = HP_OpenFile(tempFileName) < 0)) {
            BF_PrintError("Error getting block in HP_SplitFiles");
            return -1;
        }
        
        // -------------------------------------
        
        for (recordIndex = 0; recordIndex < numberOfRecordsInBlock; recordIndex++) {
            printf("record %d: %d, %s, %s, %s\n", recordIndex+1, records[recordIndex].id, records[recordIndex].name, records[recordIndex].surname, records[recordIndex].city);
            HP_InsertEntry(currentFileDesc, records[recordIndex]);
        }
    }
    
    return 0;
}

Record* bubbleSortedRecords(Record* recordsArray, const int numOfRecords, const int fieldNo) {
    int k, j;
    
    for (k = 0; k < numOfRecords; k++) {
        for (j = 0; j < numOfRecords; j++) {
            if (fieldNo == 0) {
                if (recordsArray[k].id < recordsArray[j].id) {
                    const Record tempRecord = recordsArray[j];
                    
                    recordsArray[j] = recordsArray[k];
                    recordsArray[k] = tempRecord;
                }
            }
            else if (fieldNo == 1) {
                if (strcmp(recordsArray[k].name, recordsArray[j].name) < 0) {
                    const Record tempRecord = recordsArray[j];
                    
                    recordsArray[j] = recordsArray[k];
                    recordsArray[k] = tempRecord;
                }
            }
            else if (fieldNo == 2) {
                if (strcmp(recordsArray[k].surname, recordsArray[j].surname) < 0) {
                    const Record tempRecord = recordsArray[j];
                    
                    recordsArray[j] = recordsArray[k];
                    recordsArray[k] = tempRecord;
                }
            }
            else if (fieldNo == 3) {
                if (strcmp(recordsArray[k].city, recordsArray[j].city) < 0) {
                    const Record tempRecord = recordsArray[j];
                    
                    recordsArray[j] = recordsArray[k];
                    recordsArray[k] = tempRecord;
                }
            }
        }
    }
    
    return recordsArray;
}

// ------------------------------------------------
// At first, get the current number of blocks.
// After that, check the fieldName and act as following;
// For every entry, of every block, read the current block and
// pass the read data, to struct rec. Then, having in mind the
// fieldName, print the corresponding entries.

void HP_GetAllEntries(int fileDesc, char* fieldName, void* value) {
    void* block;
    int numOfBlocks;
    
    Record rec;
    
    // -------------------------------------
    
    if ((numOfBlocks = BF_GetBlockCounter(fileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_GetAllEntries");
        return;
    }
    
    // -------------------------------------
    
    int blockIndex, recordIndex;
    
    if (strcmp(fieldName, "id") == 0) {
        int valueFound = FALSE;
        
        for (blockIndex = 1; blockIndex < numOfBlocks; blockIndex++) {
            if (BF_ReadBlock(fileDesc, blockIndex, &block) < 0) {
                BF_PrintError("Error getting block in HP_GetAllEntries");
                return;
            }
            
            int numberOfRecordsInBlock;
            memcpy(&numberOfRecordsInBlock, block , sizeof(int));
            
            for (recordIndex = 1; recordIndex <= numberOfRecordsInBlock; recordIndex++) {
                memcpy(&rec, block + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));
                
                if (rec.id == (int)value) {
                    printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                    printf("%d blocks were read\n\n------------------------\n", blockIndex);
                    
                    valueFound = TRUE;
                    
                    break;
                }
            }
            
            if (valueFound == TRUE)
                break;
        }
    }
    else {
        for (blockIndex = 1; blockIndex < numOfBlocks; blockIndex++) {
            if (BF_ReadBlock(fileDesc, blockIndex, &block) < 0) {
                BF_PrintError("Error getting block in HP_GetAllEntries");
                return;
            }
            
            int numberOfRecordsInBlock;
            memcpy(&numberOfRecordsInBlock, block , sizeof(int));
            
            for (recordIndex = 1; recordIndex <= numberOfRecordsInBlock; recordIndex++) {
                memcpy(&rec, block + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));

                if (strcmp(fieldName, "all") == 0 && rec.id > 0)
                    printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                else if (strcmp(fieldName, "name") == 0) {
                    if (strcmp(rec.name, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (strcmp(fieldName, "surname") == 0) {
                    if (strcmp(rec.surname, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (strcmp(fieldName, "city") == 0) {
                    if (strcmp(rec.city, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
            }
        }
        
        printf("%d blocks were read\n\n------------------------\n", numOfBlocks);
    }
}

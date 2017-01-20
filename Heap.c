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

int HP_DeleteFile(const char* fileName) {
    if (remove(fileName) < 0) {
        printf("\nThe %s file couldn't be deleted!", fileName);
        perror("Error");
        
        return -1;
    }
    
    return 0;
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
    int numberOfBlocks, initialHeapFileDesc;
    
    if ((initialHeapFileDesc = HP_OpenFile(initialHeapFileName) < 0)) {
        BF_PrintError("Error opening initial heap file in HP_SplitFiles");
        return -1;
    }
    
    if ((numberOfBlocks = BF_GetBlockCounter(initialHeapFileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_SplitFiles");
        return -1;
    }
    
    // -------------------------------------
    
    int blockIndex;
    
    for (blockIndex = 1; blockIndex < numberOfBlocks; blockIndex++) {
        void* block;
        int currentFileDesc;
        
        if (BF_ReadBlock(initialHeapFileDesc, blockIndex, &block) < 0) {
            BF_PrintError("Error reading block in HP_SplitFiles");
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
            BF_PrintError("Error opening temp heap file in HP_SplitFiles");
            return -1;
        }
        
        // -------------------------------------
        
        for (recordIndex = 0; recordIndex < numberOfRecordsInBlock; recordIndex++) {
            HP_InsertEntry(currentFileDesc, records[recordIndex]);
        }
        
        free(records);
        
        // -------------------------------------
        
//        if (HP_CloseFile(currentFileDesc) < 0) {
//            BF_PrintError("Error closing heap file in HP_SplitFiles");
//            return -1;
//        }
    }
    
    if (HP_CloseFile(initialHeapFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_SplitFiles");
        return -1;
    }
    
    return 0;
}

int HP_MergeFiles(char* firstFileName, char* secondFileName, const int fieldNo) {
    int firstNumberOfBlocks, secondNumberOfBlocks, blockIndex;
    int firstFileDesc, secondFileDesc;
    
    if ((firstFileDesc = HP_OpenFile(firstFileName) < 0)) {
        BF_PrintError("Error getting block in HP_MergeFiles");
        return -1;
    }
    
    if ((secondFileDesc = HP_OpenFile(secondFileName) < 0)) {
        BF_PrintError("Error getting block in HP_MergeFiles");
        return -1;
    }
    
    if ((firstNumberOfBlocks = BF_GetBlockCounter(firstFileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_MergeFiles");
        return -1;
    }
    
    if ((secondNumberOfBlocks = BF_GetBlockCounter(secondFileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_MergeFiles");
        return -1;
    }
    
    // -------------------------------------
    
    for (blockIndex = 1; blockIndex < firstNumberOfBlocks; blockIndex++) {
        void *block1, *block2;
        int currentFileDesc;
        
        if (BF_ReadBlock(firstFileDesc, blockIndex, &block1) < 0) {
            BF_PrintError("Error getting block in HP_MergeFiles");
            return -1;
        }
        
        if (BF_ReadBlock(secondFileDesc, blockIndex, &block2) < 0) {
            BF_PrintError("Error getting block in HP_MergeFiles");
            return -1;
        }
        
        int numberOfRecordsInBlockFirstFile, numberOfRecordsInBlockSecondFile;
        
        memcpy(&numberOfRecordsInBlockFirstFile, block1, sizeof(int));
        memcpy(&numberOfRecordsInBlockSecondFile, block2, sizeof(int));
        
        int recordIndex;
        
        Record* records1 = malloc(sizeof(Record) * numberOfRecordsInBlockFirstFile);
        Record* records2 = malloc(sizeof(Record) * numberOfRecordsInBlockSecondFile);
        
        Record* mergedRecords = malloc(sizeof(Record) * (numberOfRecordsInBlockFirstFile + numberOfRecordsInBlockSecondFile));
        
        for (recordIndex = 1; recordIndex <= numberOfRecordsInBlockFirstFile; recordIndex++) {
            memcpy(&records1[recordIndex-1], block1 + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));
        }
        
        for (recordIndex = 1; recordIndex <= numberOfRecordsInBlockSecondFile; recordIndex++) {
            memcpy(&records2[recordIndex-1], block2 + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));
        }
        
        mergedRecords = mergeSortedRecords(records1, records2, numberOfRecordsInBlockFirstFile, numberOfRecordsInBlockSecondFile, fieldNo);
        
        // -------------------------------------
        
        char tempFileName[15];
        strcpy(tempFileName, "merged_");
        
        char num[7];
        sprintf(num, "%d", blockIndex);
        strcat(tempFileName, num);
        
        // -------------------------------------
        
        printf("Creating %s heap file...\n", tempFileName);
        
        if (HP_CreateFile(tempFileName) < 0) {
            BF_PrintError("Error creating heap file in HP_MergeFiles");
            return -1;
        }
        
        if ((currentFileDesc = HP_OpenFile(tempFileName) < 0)) {
            BF_PrintError("Error getting block in HP_MergeFiles");
            return -1;
        }
        
        // -------------------------------------
        
        for (recordIndex = 0; recordIndex < (numberOfRecordsInBlockFirstFile + numberOfRecordsInBlockSecondFile); recordIndex++) {
            HP_InsertEntry(currentFileDesc, mergedRecords[recordIndex]);
        }
        
        free(mergedRecords);
        
        // -------------------------------------
        
        //        if (HP_CloseFile(currentFileDesc) < 0) {
        //            BF_PrintError("Error closing heap file in HP_SplitFiles");
        //            return -1;
        //        }
        
        
        if (HP_CloseFile(firstFileDesc) < 0) {
            BF_PrintError("Error closing initial heap file in HP_MergeFiles");
            return -1;
        }
        
        if (HP_CloseFile(secondFileDesc) < 0) {
            BF_PrintError("Error closing initial heap file in HP_MergeFiles");
            return -1;
        }
    }
    
    return 0;
}

Record* bubbleSortedRecords(Record* recordsArray, const int numOfRecords, const int fieldNo) {
    int k, j;
    
    if (fieldNo == 0) {
        for (k = 0; k < numOfRecords; k++) {
            for (j = 0; j < numOfRecords; j++) {
                if (recordsArray[k].id > recordsArray[j].id) {
                    const Record tempRecord = recordsArray[k];
                    
                    recordsArray[k] = recordsArray[j];
                    recordsArray[j] = tempRecord;
                }
            }
        }
    }
    else if (fieldNo == 1) {
        for (k = 0; k < numOfRecords; k++) {
            for (j = 0; j < numOfRecords; j++) {
                if (strcmp(recordsArray[k].name, recordsArray[j].name) > 0) {
                    const Record tempRecord = recordsArray[k];
                    
                    recordsArray[k] = recordsArray[j];
                    recordsArray[j] = tempRecord;
                }
            }
        }
    }
    else if (fieldNo == 2) {
        for (k = 0; k < numOfRecords; k++) {
            for (j = 0; j < numOfRecords; j++) {
                if (strcmp(recordsArray[k].surname, recordsArray[j].surname) > 0) {
                    const Record tempRecord = recordsArray[k];
                    
                    recordsArray[k] = recordsArray[j];
                    recordsArray[j] = tempRecord;
                }
            }
        }
    }
    else if (fieldNo == 3) {
        for (k = 0; k < numOfRecords; k++) {
            for (j = 0; j < numOfRecords; j++) {
                if (strcmp(recordsArray[k].city, recordsArray[j].city) > 0) {
                    const Record tempRecord = recordsArray[k];
                    
                    recordsArray[k] = recordsArray[j];
                    recordsArray[j] = tempRecord;
                }
            }
        }
    }
    
    return recordsArray;
}

Record* mergeSortedRecords(Record* Array1, Record* Array2, const int numOfRecordsFile1, const int numOfRecordsFile2, const int fieldNo) {
    int k = 0;
    int j = 0;
    int l = 0;
    
    Record* records = malloc(sizeof(Record) * (numOfRecordsFile1 + numOfRecordsFile2));
    
    while ((k < numOfRecordsFile1) || (j < numOfRecordsFile2)) {
        if (k == numOfRecordsFile1) {
            records[l] = Array2[j];
            
            j++;
            l++;
            
            if (j == numOfRecordsFile2) {
                k++;
            }
        }
        else if (j == numOfRecordsFile2) {
            records[l] = Array1[k];
            
            k++;
            l++;
            
            if (k == numOfRecordsFile1) {
                j++;
            }
        }
        else {
            if (fieldNo == 0) {
                if (Array1[k].id < Array2[j].id) {
                    records[l] = Array1[k];
                    
                    k++;
                    l++;
                }
                else if (Array1[k].id > Array2[j].id) {
                    records[l] = Array2[j];
                    
                    j++;
                    l++;
                }
                else {
                    records[l] = Array1[k];
                    records[l+1] = Array2[j];
                    
                    k++;
                    j++;
                    l += 2;
                }
            }
            else if (fieldNo == 1) {
                if (strcmp(Array1[k].name, Array2[j].name) < 0) {
                    records[l] = Array1[k];
                    
                    k++;
                    l++;
                }
                else if (strcmp(Array1[k].name, Array2[j].name) > 0) {
                    records[l] = Array2[j];
                    
                    j++;
                    l++;
                }
                else {
                    records[l] = Array1[k];
                    records[l+1] = Array2[j];
                    
                    k++;
                    j++;
                    l += 2;
                }
            }
            else if (fieldNo == 2) {
                if (strcmp(Array1[k].surname, Array2[j].surname) < 0) {
                    records[l] = Array1[k];
                    
                    k++;
                    l++;
                }
                else if (strcmp(Array1[k].surname, Array2[j].surname) > 0) {
                    records[l] = Array2[j];
                    
                    j++;
                    l++;
                }
                else {
                    records[l] = Array1[k];
                    records[l+1] = Array2[j];
                    
                    k++;
                    j++;
                    l += 2;
                }
            }
            else if (fieldNo == 3) {
                if (strcmp(Array1[k].city, Array2[j].city) < 0) {
                    records[l] = Array1[k];
                    
                    k++;
                    l++;
                }
                else if (strcmp(Array1[k].city, Array2[j].city) > 0) {
                    records[l] = Array2[j];
                    
                    j++;
                    l++;
                }	
                else {
                    records[l] = Array1[k];
                    records[l+1] = Array2[j];
                    
                    k++;
                    j++;
                    l += 2;
                }
            }
        }
    }
    
    return records;
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

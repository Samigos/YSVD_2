#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BF.h"
#include "Heap.h"

int numberOfFiles = 0;
int fileCounter = 1;
int sign = FALSE;

// ------------------------------------------------
// Create a new heap file, open it,
// allocate its first block and write
// to it the number 216, which will work
// as an ID, for heap files. After that,
// close the file.

int HP_CreateFile(char* fileName) {
    void *block;
    int fileDesc, blockNumber = SORT_FILE_ID;

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
        BF_PrintError("Error opening heap file in HP_OpenFile");
        return -1;
    }
    
    if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
        BF_PrintError("Error getting block in HP_OpenFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    // ------------------------------------------------
    
    memcpy(&blockNumber, block, sizeof(int));
    
    if (blockNumber != SORT_FILE_ID) {
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
    
    BF_PrintError("Error closing file in HP_CloseFile");

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
    numberOfFiles = numberOfBlocks-1;

    for (blockIndex = 1; blockIndex < numberOfBlocks; blockIndex++) {
        void* block;
        int currentFileDesc;
        
        if (BF_ReadBlock(initialHeapFileDesc, blockIndex, &block) < 0) {
            BF_PrintError("Error reading block in HP_SplitFiles");
            return -1;
        }
        
        // -------------------------------------
        
        int numberOfRecordsInBlock;
        memcpy(&numberOfRecordsInBlock, block, sizeof(int));
        
        Record* records = malloc(sizeof(Record) * numberOfRecordsInBlock);
        int recordIndex;

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
            BF_PrintError("Error creating temp file in HP_SplitFiles");
            return -1;
        }
        
        if ((currentFileDesc = HP_OpenFile(tempFileName)) < 0) {
            BF_PrintError("Error opening temp file in HP_SplitFiles");
            return -1;
        }
        
        // -------------------------------------
        
        for (recordIndex = 0; recordIndex < numberOfRecordsInBlock; recordIndex++) {
            HP_InsertEntry(currentFileDesc, records[recordIndex]);
        }
        
        free(records);
        
        // -------------------------------------
        
        if (HP_CloseFile(currentFileDesc) < 0) {
            BF_PrintError("Error closing temp file in HP_SplitFiles");
            return -1;
        }
    }
    
    if (HP_CloseFile(initialHeapFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_SplitFiles");
        return -1;
    }
    
    // -------------------------------------
    // Now we need to sort and merge the files
    
    int k;
    
    while (numberOfFiles >= 2) {
        if (numberOfFiles % 2 == 0) {
            for (k = 1; k < numberOfFiles; k += 2) {
                char fileName1[15], fileName2[15];
                strcpy(fileName1, "temp_");
                
                char num[7];
                sprintf(num, "%d", k);
                strcat(fileName1, num);
                
                // -------------------------------------
                
                strcpy(fileName2, "temp_");
                sprintf(num, "%d", k+1);
                strcat(fileName2, num);
                
                // -------------------------------------
                
                printf("\nCalling HP_MergeFiles, for %s and %s\n", fileName1, fileName2);
                if (HP_MergeFiles(initialHeapFileName, fileName1, fileName2, fieldNo) < 0) {
                    printf("\nError in HP_MergeFiles (1)\n");
                }
            }
            
            numberOfFiles /= 2;
        }
        else {
            sign = TRUE;
            
            char fileName1[15], fileName2[15];
            strcpy(fileName1, "temp_");
            
            char num[7];
            sprintf(num, "%d", 1);
            strcat(fileName1, num);
            
            // -------------------------------------
            
            strcpy(fileName2, "temp_");
            sprintf(num, "%d", numberOfFiles--);
            strcat(fileName2, num);
            
            // -------------------------------------
            
            printf("\nCalling HP_MergeFiles, for %s and %s\n", fileName1, fileName2);
            if (HP_MergeFiles(initialHeapFileName, fileName1, fileName2, fieldNo) < 0) {
                printf("\nError in HP_MergeFiles (2)\n");
            }
        }
    }
    
    return 0;
}

int HP_MergeFiles(char* initialHeapFileName, char* firstFileName, char* secondFileName, const int fieldNo) {
    int firstNumberOfBlocks, secondNumberOfBlocks, blockIndex;
    int firstFileDesc, secondFileDesc;
    int recordIndex;
    
    if ((firstFileDesc = HP_OpenFile(firstFileName)) < 0) {
        BF_PrintError("Error opening first merged temp file in HP_MergeFiles");
        return -1;
    }
    
    if ((secondFileDesc = HP_OpenFile(secondFileName)) < 0) {
        BF_PrintError("Error opening second merged temp file in HP_MergeFiles");
        return -1;
    }
    
    if ((firstNumberOfBlocks = BF_GetBlockCounter(firstFileDesc)) < 0) {
        BF_PrintError("Error getting block counter of first merged temp file in HP_MergeFiles");
        return -1;
    }
    
    if ((secondNumberOfBlocks = BF_GetBlockCounter(secondFileDesc)) < 0) {
        BF_PrintError("Error getting block counter of second merged temp file in HP_MergeFiles");
        return -1;
    }
    
    // -------------------------------------
    
    int tempFileDesc;
    
    printf("Creating helping temp heap file...\n");
    
    if (HP_CreateFile("temp") < 0) {
        BF_PrintError("Error creating merged heap file in HP_MergeFiles");
        return -1;
    }
    
    if ((tempFileDesc = HP_OpenFile("temp")) < 0) {
        BF_PrintError("Error opening temp file in HP_MergeFiles");
        return -1;
    }
    
    // -------------------------------------
    
    for (blockIndex = 1; blockIndex < firstNumberOfBlocks; blockIndex++) {
        void* block1, *block2;
        
        printf("%s, %s - firstNumberOfBlocks %d, secondNumberOfBlocks %d\n", firstFileName, secondFileName, firstNumberOfBlocks, secondNumberOfBlocks);
        
        if (BF_ReadBlock(firstFileDesc, blockIndex, &block1) < 0) {
            BF_PrintError("Error getting block of first merged temp file in HP_MergeFiles");
            return -1;
        }
        
        if (blockIndex < secondNumberOfBlocks) {
            if (BF_ReadBlock(secondFileDesc, blockIndex, &block2) < 0) {
                BF_PrintError("Error getting block of second merged temp file in HP_MergeFiles");
                return -1;
            }
        }
        
        int numberOfRecordsInBlockFirstFile = 0, numberOfRecordsInBlockSecondFile = 0;

        memcpy(&numberOfRecordsInBlockFirstFile, block1, sizeof(int));
        memcpy(&numberOfRecordsInBlockSecondFile, block2, sizeof(int));
        
        // -------------------------------------
        
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
        
        for (recordIndex = 0; recordIndex < (numberOfRecordsInBlockFirstFile + numberOfRecordsInBlockSecondFile); recordIndex++) {
            HP_InsertEntry(tempFileDesc, mergedRecords[recordIndex]);
        }
        
        free(mergedRecords);
    }
    
    if (HP_CloseFile(firstFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_MergeFiles");
        return -1;
    }
    
    if (HP_CloseFile(secondFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_MergeFiles");
        return -1;
    }
    
    printf("Deleting %s...\n\n", firstFileName);
    
    if (HP_DeleteFile(firstFileName) < 0) {
        printf("Error deleting temp heap file %s in HP_MergeFiles", firstFileName);
    }
    
    printf("Deleting %s...\n\n", secondFileName);
    
    if (HP_DeleteFile(secondFileName) < 0) {
        printf("Error deleting temp heap file %s in HP_MergeFiles", secondFileName);
    }
    
    // -------------------------------------
    
    char tempFileName[30];
    char num[7];
    
    if (numberOfFiles == 2) {
        strcpy(tempFileName, initialHeapFileName);
        strcat(tempFileName, "Sorted");
        
        sprintf(num, "%d", fieldNo);
        strcat(tempFileName, num);
    }
    else {
        strcpy(tempFileName, "temp_");
        
        if (sign == TRUE) {
            sign = FALSE;
            
            sprintf(num, "%d", 1);
            strcat(tempFileName, num);
        }
        else {
            sprintf(num, "%d", fileCounter++);
            strcat(tempFileName, num);
        }
        
        if (numberOfFiles % 2 == 0 && fileCounter >= (numberOfFiles / 2) + 1) {
            fileCounter = 1;
        }
        else if (numberOfFiles % 2 != 0 && fileCounter >= (numberOfFiles / 2) + 2) {
            fileCounter = 1;
        }
    }
    
    // -------------------------------------
    
    printf("Creating new %s file\n", tempFileName);

    void* block;
    int newFileDesc, finalNumberOfBlocks;
    
    if (HP_CreateFile(tempFileName) < 0) {
        BF_PrintError("Error creating merged heap file in HP_MergeFiles");
        return -1;
    }
    
    if ((newFileDesc = HP_OpenFile(tempFileName)) < 0) {
        BF_PrintError("Error opening temp file in HP_MergeFiles");
        return -1;
    }
    
    if ((finalNumberOfBlocks = BF_GetBlockCounter(tempFileDesc)) < 0) {
        BF_PrintError("Error getting final block counter in HP_MergeFiles");
        return -1;
    }
    
    // -------------------------------------
    
    for (blockIndex = 0; blockIndex < finalNumberOfBlocks; blockIndex++) {
        if (BF_ReadBlock(tempFileDesc, blockIndex, &block) < 0) {
            BF_PrintError("Error getting block of first merged temp file in HP_MergeFiles zzz");
            return -1;
        }
        
        if (blockIndex == 0) {
            int fileId;
            memcpy(&fileId, block, sizeof(int));
            
            // ------------------------------------------------
            
            if (BF_AllocateBlock(newFileDesc) < 0) {
                BF_PrintError("Error allocating block in HP_CreateFile");
                BF_CloseFile(newFileDesc);
                
                return -1;
            }
            
            if (BF_ReadBlock(newFileDesc, 0, &block) < 0) {
                BF_PrintError("Error getting block in HP_CreateFile");
                BF_CloseFile(newFileDesc);
                
                return -1;
            }
            
            memcpy(block, &fileId, sizeof(int));
            
            // ------------------------------------------------
            
            if (BF_WriteBlock(newFileDesc, 0) < 0) {
                BF_PrintError("Error writing to block in HP_CreateFile");
                BF_CloseFile(newFileDesc);
                
                return -1;
            }
        }
        else {
            int numberOfRecords = 0;
            memcpy(&numberOfRecords, block, sizeof(int));

            int recordIndex;

            for (recordIndex = 1; recordIndex <= numberOfRecords; recordIndex++) {
                Record rec;
                memcpy(&rec, block + sizeof(int) + (recordIndex * sizeof(Record)), sizeof(Record));

                HP_InsertEntry(newFileDesc, rec);
            }
        }
    }
    
    HP_GetAllEntries(newFileDesc, 1, NULL);
    
    if (HP_CloseFile(newFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_MergeFiles");
        return -1;
    }
    
    // -------------------------------------
    
    if (HP_CloseFile(tempFileDesc) < 0) {
        BF_PrintError("Error closing initial heap file in HP_MergeFiles");
        return -1;
    }
    
    if (HP_DeleteFile("temp") < 0) {
        printf("Error deleting temp heap file in HP_MergeFiles");
    }
    
    // -------------------------------------
    
    return 0;
}

Record* bubbleSortedRecords(Record* recordsArray, const int numOfRecords, const int fieldNo) {
    int k, j;
    
    if (fieldNo == 0) {
        for (k = 0; k < numOfRecords; k++) {
            for (j = 0; j < numOfRecords; j++) {
                if (recordsArray[k].id < recordsArray[j].id) {
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
                if (strcmp(recordsArray[k].name, recordsArray[j].name) < 0) {
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
                if (strcmp(recordsArray[k].surname, recordsArray[j].surname) < 0) {
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
                if (strcmp(recordsArray[k].city, recordsArray[j].city) < 0) {
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
    Record* sortedRecordsArray = malloc(sizeof(Record) * (numOfRecordsFile1 + numOfRecordsFile2));
    int firstArrayIndex = 0, secondArrayIndex = 0, sortedArrayIndex = 0;
    
    while ((firstArrayIndex < numOfRecordsFile1) || (secondArrayIndex < numOfRecordsFile2)) {
        if (firstArrayIndex == numOfRecordsFile1) {
            sortedRecordsArray[sortedArrayIndex] = Array2[secondArrayIndex];
            secondArrayIndex++;
            
            if (secondArrayIndex == numOfRecordsFile2) {
                break;
            }
        }
        else if (secondArrayIndex == numOfRecordsFile2) {
            sortedRecordsArray[sortedArrayIndex] = Array1[firstArrayIndex];
            firstArrayIndex++;
            
            if (firstArrayIndex == numOfRecordsFile1) {
                break;
            }
        }
        else {
            if (fieldNo == 0) {
                if (Array1[firstArrayIndex].id < Array2[secondArrayIndex].id) {
                    sortedRecordsArray[sortedArrayIndex] = Array1[firstArrayIndex];
                    firstArrayIndex++;
                }
                else if (Array1[firstArrayIndex].id > Array2[secondArrayIndex].id) {
                    sortedRecordsArray[sortedArrayIndex] = Array2[secondArrayIndex];
                    secondArrayIndex++;
                }
                else {
                    sortedRecordsArray[sortedArrayIndex] = Array1[firstArrayIndex];
                    sortedRecordsArray[sortedArrayIndex+1] = Array2[secondArrayIndex];
                    
                    firstArrayIndex++;
                    secondArrayIndex++;
                    sortedArrayIndex++;
                }
            }
            else {
                char* attr1, *attr2;
                
                if (fieldNo == 1) {
                    attr1 = malloc(strlen(Array1[firstArrayIndex].name) + 1);
                    attr2 = malloc(strlen(Array2[secondArrayIndex].name) + 1);
                }
                else if (fieldNo == 2) {
                    attr1 = malloc(strlen(Array1[firstArrayIndex].surname) + 1);
                    attr2 = malloc(strlen(Array2[secondArrayIndex].surname) + 1);
                }
                else {
                    attr1 = malloc(strlen(Array1[firstArrayIndex].city) + 1);
                    attr2 = malloc(strlen(Array2[secondArrayIndex].city) + 1);
                }
                
                if (strcmp(attr1, attr2) < 0) {
                    sortedRecordsArray[sortedArrayIndex] = Array1[firstArrayIndex];
                    firstArrayIndex++;
                }
                else if (strcmp(attr1, attr2) > 0) {
                    sortedRecordsArray[sortedArrayIndex] = Array2[secondArrayIndex];
                    secondArrayIndex++;
                }
                else {
                    sortedRecordsArray[sortedArrayIndex] = Array1[firstArrayIndex];
                    sortedRecordsArray[sortedArrayIndex+1] = Array2[secondArrayIndex];
                    
                    firstArrayIndex++;
                    secondArrayIndex++;
                    sortedArrayIndex++;
                }
            }
        }
        
        sortedArrayIndex++;
    }
    
    return sortedRecordsArray;
}

// ------------------------------------------------
// At first, get the current number of blocks.
// After that, check the fieldName and act as following;
// For every entry, of every block, read the current block and
// pass the read data, to struct rec. Then, having in mind the
// fieldName, print the corresponding entries.

void HP_GetAllEntries(int fileDesc, int fieldNo, void* value) {
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
    
    if (fieldNo == 0) {
        int valueFound = FALSE;
        
        for (blockIndex = 1; blockIndex < numOfBlocks; blockIndex++) {
            if (BF_ReadBlock(fileDesc, blockIndex, &block) < 0) {
                BF_PrintError("Error getting block in HP_GetAllEntries");
                return;
            }
            
            int numberOfRecordsInBlock;
            memcpy(&numberOfRecordsInBlock, block, sizeof(int));
            
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

                if (value == NULL)
                    printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                else if (fieldNo == 1) {
                    if (strcmp(rec.name, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (fieldNo == 2) {
                    if (strcmp(rec.surname, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (fieldNo == 3) {
                    if (strcmp(rec.city, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
            }
        }
        
        printf("%d blocks were read\n\n------------------------\n", numOfBlocks);
    }
}

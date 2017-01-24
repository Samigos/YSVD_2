#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BF.h"
#include "Sort.h"
#include "Heap.h"

#define fileName "heapFile"

void insert_Entries(int);

int main(int argc, char **argv) {
    int fd;
    int fieldNo = 0;
   
    BF_Init();
    
    //create heap file
    if (Sorted_CreateFile(fileName) == -1)
        printf("Error creating file in main!\n");
    
    fd = Sorted_OpenFile(fileName);
    
    if (fd == -1)
        printf("Error opening file in main!\n");
    
    insert_Entries(fd);
    //Sorted_GetAllEntries(fd, &fieldNo, NULL);

    if (Sorted_CloseFile(fd) < 0) {
        printf("Error closing file in main!\n");
    }
    
    //sort heap file using 2-way merge-sort
    Sorted_SortFile(fileName, 0);
 //   Sorted_checkSortedFile("heapFileSorted", fieldNo);
    
    //get all entries with value
    //char value[20];
    //strcpy(value, "Keratsini");
    
    //int value = 11903588;
 
    fd = Sorted_OpenFile("heapFileSorted0");
    
    if( fd == -1  )
        printf("Error opening file!\n");
    
    printf("\n\n\n");
    Sorted_GetAllEntries(fd, &fieldNo, NULL);
    
    return EXIT_SUCCESS;
}

void insert_Entries(int fd) {
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    stream = stdin;
    Record record;

    while ((read = getline(&line, &len, stream)) != -1) {
        line[read - 2] = 0;
        char *pch;

        pch = strtok(line, ",");
        record.id = atoi(pch);

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.name, pch, sizeof(record.name));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.surname, pch, sizeof(record.surname));

        pch = strtok(NULL, ",");
        pch++;
        pch[strlen(pch) - 1] = 0;
        strncpy(record.city, pch, sizeof(record.city));

        assert(!Sorted_InsertEntry(fd, record));
    }
    
    free(line);
}

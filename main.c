#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BF.h"
#include "Sorted.h"

#define fileName "heapFile"

void insert_Entries(int);

int main(int argc, char **argv) {
    int fd;
    int fieldNo;
   
    BF_Init();
    
    //create heap file
    if (Sorted_CreateFile(fileName) == -1)
        printf("Error creating file!\n");
    
    fd = Sorted_OpenFile(fileName);
    
    if (fd == -1)
        printf("Error opening file!\n");
    
    insert_Entries(fd);
    
    //sort heap file using 2-way merge-sort
    if (Sorted_SortFile(fileName, 0) == -1)
        printf("Error sorting file!\n");
    
    if (Sorted_checkSortedFile("heapFileSorted", 0) == -1)
        printf("Error sorting file!\n");
    
    //get all entries with value
    //char value[20];
    //strcpy(value, "Keratsini");
    
    fieldNo = 0;
    int value = 11903588;
    
    fd = Sorted_OpenFile("heapFileSorted0");
    
    if( fd == -1  )
        printf("Error opening file!\n");
    
    Sorted_GetAllEntries(fd, &fieldNo, &value);
    
    return EXIT_SUCCESS;
}

void insert_Entries(int fd) {
    //TO DO
    //read from input stream and write on fd
}

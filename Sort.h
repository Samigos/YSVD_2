//
//  Sort.h
//  YSVD_2
//
//  Created by Sotiris Kaniras on 14/01/2017.
//  Copyright © 2017 com.me. All rights reserved.
//

#ifndef Sort_h
#define Sort_h

typedef struct Record {
    int id;
    char name[15];
    char surname[20];
    char city[25];
} Record;

int Sorted_CreateFile(char*);
int Sorted_OpenFile(char*);
int Sorted_CloseFile(int);
int Sorted_InsertEntry(int, Record);
void Sorted_SortFile(char*, int);
void Sorted_checkSortedFile(char*, int);
void Sorted_GetAllEntries(int, int*, void*);

#endif /* Sort_h */

#ifndef Heap_h
#define Heap_h

#define HEAP_FILE_ID 216250 // Special information for the first block of a heap file

#define TRUE 1
#define FALSE 0

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[25];
} Record;

int HP_CreateFile(char* fileName);
int HP_OpenFile(char* fileName);
int HP_CloseFile(int fileDesc);
int HP_InsertEntry(int fileDesc, Record record);
void HP_GetAllEntries(int fileDesc, char* fieldName, void* value);
int HP_SplitFiles(char*, const int);
int HP_DeleteFile(const char*);
int HP_MergeFiles(char*, char*, const int);


Record* bubbleSortedRecords(Record*, const int, const int);
Record* Mergesort(Record*, Record*, const int, const int, const int);

#endif

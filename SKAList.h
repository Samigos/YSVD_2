#ifndef SKAList_h
#define SKAList_h

typedef struct SKAList {
    struct SKAList* current, *next, *last;
    int value;
} SKAList;

void appendToList(SKAList**, const int);
void printList(SKAList*);

#endif

#include <stdio.h>
#include <stdlib.h>
#include "SKAList.h"

// -----------------------------------------------------------------------
// If the list is empty, allocate it and make
// the current node, point to the newly created
// node (temp). Else, do the following:

// 1) Keep the current node of the list,
// so when we are done, we will make the list
// point to its start

// 2) Go to the end of the list, where the new
// node will be added

// 3) Add the new node to the end of the list,
// make the next of the current node point to
// the last and finally, point again to the start
// of the list.

void appendToList(SKAList** list, const int newValue) {
    SKAList* temp = malloc(sizeof(SKAList));
    temp->value = newValue;
    
    if (*list == NULL) { // list is empty
        *list = malloc(sizeof(SKAList));
        (*list)->current = temp;
    }
    else { // list is not empty
        SKAList* first = (*list)->current;

        while ((*list)->current->next != NULL) {
            (*list)->current = (*list)->current->next;
        }
        
        (*list)->last = temp;
        (*list)->current->next = (*list)->last;
        (*list)->current = first;
    }
}

// -----------------------------------------------------------------------
// If list points to NULL, it means that nothing has been inserted yet,
// therefore it's empty. Else, keep the current node of the list, so when
// we are done, we will make the list point to its start. After that,
// iterate through the whole list, from next-to-next and print the value.
// Lastly, point again to the start of the list.

void printList(SKAList* list) {
    if (list == NULL) {
        printf("The list is empty\n");
    }
    else {
        SKAList* first = list->current;

        while (list->current != NULL) {
            printf("%d ", list->current->value);
            list->current = list->current->next;
        }
        
        list->current = first;
    }
    
    printf("\n");
}

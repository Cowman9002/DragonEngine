#ifndef C_LINKED_LIST_H
#define C_LINKED_LIST_H

#include <stdlib.h>

#ifndef C_LINKED_LIST_INTERNAL
typedef void LinkedList;
#endif // C_ORDERED_MAP_INTERNAL

LinkedList *LinkedListCreate();
void LinkedListDestroy(LinkedList *list);

void LinkedListPushBack(LinkedList *list, void *value, size_t sizeof_value);
void LinkedListInsert(LinkedList *list, size_t location, void *value, size_t sizeof_value);

void *LinkedListAt(LinkedList *list, size_t location);

#endif // C_LINKED_LIST_H

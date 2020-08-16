#ifndef C_LINKED_LIST_H
#define C_LINKED_LIST_H

#include <stddef.h>

#ifndef C_LINKED_LIST_INTERNAL
typedef void LinkedList;
#endif // C_ORDERED_MAP_INTERNAL

LinkedList *linkedListCreate();
void linkedListDestroy(LinkedList *list);

void linkedListPushBack(LinkedList *list, void *value, size_t sizeof_value);
void linkedListPushFront(LinkedList *list, void *value, size_t sizeof_value);
void linkedListInsert(LinkedList *list, size_t location, void *value, size_t sizeof_value);

void *linkedListAt(LinkedList *list, size_t location);

size_t linkedListGetCount(LinkedList *list);

#endif // C_LINKED_LIST_H

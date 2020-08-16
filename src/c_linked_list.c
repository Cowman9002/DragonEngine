#include <stdint.h>
#include <MemLeaker/malloc.h>

typedef struct ListElement
{
    void *value;
    struct ListElement *next;
    size_t value_size;
}ListElement;


typedef struct
{
    ListElement *head;
    size_t count;
}LinkedList;

#define C_LINKED_LIST_INTERNAL
#include "c_linked_list.h"

ListElement *getListElement_internal(LinkedList *list, size_t location);

LinkedList *linkedListCreate()
{
    LinkedList *res = malloc(sizeof(*res));

    res->head = NULL;
    res->count = 0;

    return res;
}

void linkedListDestroy(LinkedList *list)
{
    ListElement *curr_element = list->head;
    ListElement *next_element = NULL;
    for(int i = 0; i < list->count; i++)
    {
        next_element = curr_element->next;

        // free current element's values
        //FIXME This will eventually break
        free(curr_element->value);

        free(curr_element);

        curr_element = next_element;
    }

    free(list);
    //values are malloced
    //elements are malloced
    //list is malloced
}

void linkedListPushBack(LinkedList *list, void *value, size_t sizeof_value)
{
    if(list->count == 0)
    {
        linkedListPushFront(list, value, sizeof_value);
        return;
    }

    if(list == NULL) return;

    // create a new list element and set its next to NULL
    ListElement *new_end = malloc(sizeof(*new_end));
    new_end->next = NULL;
    // set value in new element to value
    new_end->value = malloc(sizeof_value);
    new_end->value_size = sizeof_value;

    for(size_t i = 0; i < sizeof_value; i++)
    {
        *(int8_t*)(new_end->value + i) = *(int8_t*)(value + i);
    }

    // get last element in the list and set its next to new node
    getListElement_internal(list, list->count - 1)->next = new_end;
    list->count++;
}

void linkedListPushFront(LinkedList *list, void *value, size_t sizeof_value)
{
    if(list == NULL) return;

    // create a new list element and set its next to the head node
    ListElement *new_head = malloc(sizeof(*new_head));
    new_head->next = list->head;
    // set value in new element to value
    new_head->value = malloc(sizeof_value);
    new_head->value_size = sizeof_value;

    for(size_t i = 0; i < sizeof_value; i++)
    {
        *(int8_t*)(new_head->value + i) = *(int8_t*)(value + i);
    }
    // set head node to new list element
    list->head = new_head;
    list->count++;
}

void linkedListInsert(LinkedList *list, size_t location, void *value, size_t sizeof_value)
{

}

void *linkedListAt(LinkedList *list, size_t location)
{
    ListElement *target = getListElement_internal(list, location);
    if(target == NULL) return NULL;

    return target->value;
}

size_t linkedListGetCount(LinkedList *list)
{
    if(list == NULL) return 0;
    return list->count;
}

ListElement *getListElement_internal(LinkedList *list, size_t location)
{
    if(list == NULL) return NULL;
    if(location >= list->count) return NULL;

    ListElement *target = list->head;

    // work up the list to the target location
    for(int i = 0; i < location; i++)
    {
        target = target->next;
    }

    return target;
}

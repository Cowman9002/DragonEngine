#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <MemLeaker/malloc.h>

typedef struct
{
    const char *key;
    void *value;
    size_t value_size;
}MapElementS;

typedef struct
{
    MapElementS *elements;
    size_t count;
}OrderedMapS;

#define C_ORDERED_MAP_INTERNAL
#include "c_ordered_map.h"

uint8_t orderedMapSAtIndex_internal(OrderedMapS *map, size_t i, MapElementS *out_element);
uint8_t orderedMapSAtKey_internal(OrderedMapS *map, const char *key, MapElementS *out_element, size_t *out_index);
void orderedMapSInsert_internal(OrderedMapS *map, const char *key, void *value, size_t sizeof_value, uint8_t replace);

OrderedMapS *orderedMapSCreate()
{
    OrderedMapS *res = malloc(sizeof(*res));

    if(res == NULL)
    {
        return NULL;
    }

    res->elements = NULL;
    res->count = 0;

    return res;
}

void orderedMapSDestroy(OrderedMapS *map)
{
    if(map == NULL) return;
    orderedMapSClear(map);
    free(map);
}

void orderedMapSInsert(OrderedMapS *map, const char *key, void *value, size_t sizeof_value)
{
    orderedMapSInsert_internal(map, key, value, sizeof_value, 0);
}

void orderedMapSInsertOrReplace(OrderedMapS *map, const char *key, void *value, size_t sizeof_value)
{
    orderedMapSInsert_internal(map, key, value, sizeof_value, 1);
}

void orderedMapSEraseAtKey(OrderedMapS *map, const char *key)
{
    MapElementS e;
    size_t i = 0;

    if(orderedMapSAtKey_internal(map, key, &e, &i))
    {
        free(e.value);
        map->count--;
        for(size_t j = i; j < map->count; j++)
        {
            map->elements[j] = map->elements[j + 1];
        }
        map->elements = realloc(map->elements, sizeof(*(map->elements)) * map->count);
    }
}

void orderedMapSEraseAtIndex(OrderedMapS *map, size_t i)
{
    MapElementS e;

    if(orderedMapSAtIndex_internal(map, i, &e))
    {
        free(e.value);
        map->count--;
        for(size_t j = i; j < map->count; j++)
        {
            map->elements[j] = map->elements[j + 1];
        }
        map->elements = realloc(map->elements, sizeof(MapElementS) * map->count);
    }
}

void orderedMapSClear(OrderedMapS *map)
{
    if(map == NULL) return;

    for(int i = 0; i < map->count; i++)
    {
        // FIXME This will eventually break
        free(map->elements[i].value);
    }
    free(map->elements);
}

void *orderedMapSAtKey(OrderedMapS *map, const char *key)
{
    MapElementS e;

    if(orderedMapSAtKey_internal(map, key, &e, 0))
    {
        return e.value;
    }

    return NULL;
}

void *orderedMapSAtIndex(OrderedMapS *map, size_t i)
{
    MapElementS e;

    if(orderedMapSAtIndex_internal(map, i, &e))
    {
        return e.value;
    }

    return 0;
}

const char *orderedMapSKeyAtIndex(OrderedMapS *map, size_t i)
{
    MapElementS e;

    if(orderedMapSAtIndex_internal(map, i, &e))
    {
        return e.key;
    }

    return NULL;
}

size_t orderedMapSGetCount(OrderedMapS *map)
{
    if(map == NULL) return 0;

    return map->count;
}

uint8_t orderedMapSAtKey_internal(OrderedMapS *map, const char *key, MapElementS *out_element, size_t *out_index)
{
    if(map == NULL) return 0;
    if(map->elements == NULL) return 0;

    size_t lower_bound = 0;
    size_t upper_bound = map->count;
    upper_bound = upper_bound > 0 ? upper_bound - 1 : 0;
    size_t current_index = 0;
    size_t neighbor_index = 0;

    // go to middle
    // check two surrounding values
    //      special cases for ends
    // repeat until bottom bound and top bound are the same

    while(1)
    {

        // ints always truncate, or floor for unsigned
        // average top and bottom bound to find midpoint
        current_index = (lower_bound + upper_bound) / 2;
        neighbor_index = current_index + 1;

        int cmp1 = 0;
        int cmp2 = 0;

        cmp1 = strcmp(key, map->elements[current_index].key);

        if(neighbor_index >= map->count) // at end of array
        {
            cmp2 = -1;
        }
        else
        {
            cmp2 = strcmp(key, map->elements[neighbor_index].key);
        }

        if(cmp1 > 0 && cmp2 < 0) // goes between the values, does not exist
        {
            return 0;
        }
        else if(cmp1 > 0 && cmp2 > 0) // goes later in the list
        {
            lower_bound = neighbor_index;
        }
        else if(cmp1 < 0 && cmp2 < 0) // goes later in the list
        {
            upper_bound = current_index;
        }
        else if(cmp1 == 0 || cmp2 == 0)  // value correct value
        {
            size_t target_index = cmp1 == 0 ? current_index : neighbor_index;

            if(out_element)
                *out_element = map->elements[target_index];
            if(out_index)
                *out_index = target_index;
            return 1;
        }
    }

    return 0;
}

uint8_t orderedMapSAtIndex_internal(OrderedMapS *map, size_t i, MapElementS *out_element)
{
    if(map == NULL) return 0;

    //bounds check
    if(i < 0 || i >= map->count)
    {
        return 0;
    }

    if(out_element)
        *out_element = map->elements[i];
    return 1;
}

void orderedMapSInsert_internal(OrderedMapS *map, const char *key, void *value, size_t sizeof_value, uint8_t replace)
{
    if(map == NULL) return;

    size_t lower_bound = 0;
    size_t upper_bound = map->count;
    upper_bound = upper_bound > 0 ? upper_bound - 1 : 0;
    size_t current_index = 0;
    size_t neighbor_index = 0;
    size_t target_index = 0;

    MapElementS element;
    element.key = key;
    element.value_size = sizeof_value;
    element.value = malloc(sizeof_value);

    for(size_t i = 0; i < sizeof_value; i++)
    {
        *(int8_t*)(element.value + i) = *(int8_t*)(value + i);
    }

    // go to middle
    // check two surrounding values
    //      special cases for ends
    // repeat until bottom bound and top bound are the same

    while(1)
    {
        // at the beginning of array
        if(upper_bound == lower_bound && upper_bound == 0)
        {
            target_index = 0;
            break;
        }

        // ints always truncate, or floor for unsigned
        // average top and bottom bound to find midpoint
        current_index = (lower_bound + upper_bound) / 2;
        neighbor_index = current_index + 1;

        int cmp1 = 0;
        int cmp2 = 0;

        if(neighbor_index >= map->count) // at end of array
        {
            cmp2 = -1;
        }
        else
        {
            cmp2 = strcmp(key, map->elements[neighbor_index].key);
        }

        cmp1 = strcmp(key, map->elements[current_index].key);

        if(cmp1 > 0 && cmp2 < 0) // goes between the values, correct index
        {
            target_index = neighbor_index;
            break;
        }
        else if(cmp1 > 0 && cmp2 > 0) // goes later in the list
        {
            lower_bound = neighbor_index;
        }
        else if(cmp1 < 0 && cmp2 < 0) // goes later in the list
        {
            upper_bound = current_index;
        }
        else if(cmp1 == 0 || cmp2 == 0)  // value already exists
        {
            if(replace)
            {
                target_index = cmp1 == 0 ? current_index : neighbor_index;
                free(map->elements[target_index].value);
                map->elements[target_index] = element;
                return;
            }
            else
            {
                return;
            }
        }
    }


    // set the array at correct index to key-value pair

    // make room for the new element
    map->elements = realloc(map->elements, sizeof(MapElementS) * (map->count + 1));

    // shift all elements after position over one
    for(size_t i = map->count; i > target_index; i--)
    {
        map->elements[i] = map->elements[i - 1];
    }

    map->elements[target_index] = element;
    map->count++;
}

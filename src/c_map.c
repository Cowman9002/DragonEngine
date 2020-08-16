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

OrderedMapS *orderedMapSCreate()
{
    OrderedMapS *res = malloc(sizeof(*res));
    res->elements = NULL;
    res->count = 0;

    return res;
}

void orderedMapSDestroy(OrderedMapS *map)
{
    if(map == NULL) return;
    for(int i = 0; i < map->count; i++)
    {
        // FIXME This will eventually break
        free(map->elements[i].value);
    }

    free(map);
}

void orderedMapSInsert(OrderedMapS *map, const char *key, void *value, size_t sizeof_value)
{
    if(map == NULL) return;

    /*if(map->count == 0)
    {
        map->elements = realloc(map->elements, sizeof(MapElementS));

        map->elements[0] = element;
        map->count++;
    }
    else if(map->count == 1)
    {
        int cmp = strcmp(key, map->elements[current_index].key);

        map->elements = realloc(map->elements, sizeof(MapElementS) * 2);

        map->elements[0] = element;
        map->count++;
    }*/

    size_t lower_bound = 0;
    size_t upper_bound = map->count;
    upper_bound = upper_bound > 0 ? upper_bound - 1 : 0;
    size_t current_index = 0;
    size_t neighbor_index = 0;

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
        // ints always truncate, or floor for unsigned
        // average top and bottom bound to find midpoint
        current_index = (lower_bound + upper_bound) / 2;
        neighbor_index = current_index + 1;

    }

    // set the array at correct index to key-value pair

    // make room for the new element
    map->elements = realloc(map->elements, sizeof(MapElementS) * (map->count + 1));

    // shift all elements after position over one
    for(size_t i = map->count; i > lower_bound; i--)
    {
        map->elements[i] = map->elements[i - 1];
    }

    map->elements[lower_bound] = element;
    map->count++;
}

void orderedMapSInsertOrReplace(OrderedMapS *map, const char *key, void *value)
{
    if(map == NULL) return;
}

void orderedMapSErase(OrderedMapS *map)
{
    if(map == NULL) return;
}

void orderedMapSClear(OrderedMapS *map)
{
    if(map == NULL) return;
}

void *orderedMapSAtKey(OrderedMapS *map, const char *key)
{
    if(map == NULL) return 0;
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

    *out_element = map->elements[i];
    return 1;
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

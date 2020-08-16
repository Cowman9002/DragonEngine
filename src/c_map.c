#include <string.h>
#include <math.h>
#include <stdint.h>
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

OrderedMapS *OrderedMapSCreate()
{
    OrderedMapS *res = malloc(sizeof(*res));
    res->elements = NULL;
    res->count = 0;

    return res;
}

void OrderedMapSDestroy(OrderedMapS *map)
{
    if(map == NULL) return;
    for(int i = 0; i < map->count; i++)
    {
        MapElementS e = map->elements[i];
        // free one char at a time
        for(int j = 0; j < e.value_size; j++)
        {
            free(map->elements[i].value + j);
        }
    }
}

void OrderedMapSInsert(OrderedMapS *map, const char *key, void *value, size_t sizeof_value)
{
    if(map == NULL) return;

    size_t lower_bound = 0;
    size_t upper_bound = map->count;
    size_t current_index = 0;

    MapElementS element;
    element.key = key;
    element.value_size = sizeof_value;
    element.value = malloc(sizeof_value);

    for(size_t i = 0; i < sizeof_value; i++)
    {
        *(int8_t*)(element.value + i) = *(int8_t*)(value + i);
    }


    element.value = value;

    // go to middle
    //      if strcmp > 0 go to middle of first half
    //      if strcmp < 0 go to middle of second half
    //      if strcmp == 0 return void
    // repeat until bottom bound and top bound are the same

    while(1)
    {
        // ints always truncate, or floor for unsigned
        if(lower_bound != map->count - 2 && upper_bound != map->count - 1 )
        {
            // average top and bottom bound to find midpoint
            current_index = (lower_bound + upper_bound) / 2;
        }
        else
        {
            // avoids being stuck on (count - 1) and (count - 2)
            current_index = map->count - 1;
        }

        if(current_index >= map->count || current_index < 0) break;

        int cmp = strcmp(key, map->elements[current_index].key);

        if(cmp < 0) // key comes before midpoint
        {
            if(lower_bound == upper_bound) // break check
            {
                break;
            }
            upper_bound = current_index;
        }
        else if(cmp > 0) // key comes after midpoint
        {
            if(lower_bound == upper_bound) // break check
            {
                if(lower_bound == map->count - 1) // needs to be added after
                {
                    lower_bound = map->count;
                    upper_bound = lower_bound;
                }
                break;
            }

            lower_bound = current_index;
        }
        else // key already exists
        {
            return;
        }
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

void OrderedMapSInsertOrReplace(OrderedMapS *map, const char *key, void *value)
{
    if(map == NULL) return;
}

void OrderedMapSErase(OrderedMapS *map)
{
    if(map == NULL) return;
}

void OrderedMapSClear(OrderedMapS *map)
{
    if(map == NULL) return;
}

int OrderedMapSAtKeyI(OrderedMapS *map, const char *key)
{
    if(map == NULL) return 0;
    return 0;
}

uint8_t OrderedMapSAtIndex_internal(OrderedMapS *map, size_t i, MapElementS *out_element)
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

int OrderedMapSAtIndexI(OrderedMapS *map, size_t i)
{
    MapElementS e;

    if(OrderedMapSAtIndex_internal(map, i, &e))
    {
        return *(int *)e.value;
    }

    return 0;
}

const char *OrderedMapSKeyAtIndex(OrderedMapS *map, size_t i)
{
    MapElementS e;

    if(OrderedMapSAtIndex_internal(map, i, &e))
    {
        return e.key;
    }

    return NULL;
}

size_t OrderedMapSGetCount(OrderedMapS *map)
{
    if(map == NULL) return 0;

    return map->count;
}

#ifndef C_ORDERED_MAP_H
#define C_ORDERED_MAP_H

#include <stddef.h>

#ifndef C_ORDERED_MAP_INTERNAL
typedef void OrderedMapS;
#endif // C_ORDERED_MAP_INTERNAL

OrderedMapS *orderedMapSCreate();
void orderedMapSDestroy(OrderedMapS *map);
void orderedMapSInsert(OrderedMapS *map, const char *key, void *value, size_t sizeof_value);
void orderedMapSInsertOrReplace(OrderedMapS *map, const char *key, void *value);
void orderedMapSErase(OrderedMapS *map);
void orderedMapSClear(OrderedMapS *map);

void *orderedMapSAtKey(OrderedMapS *map, const char *key);
void *orderedMapSAtIndex(OrderedMapS *map, size_t i);

const char *orderedMapSKeyAtIndex(OrderedMapS *map, size_t i);

size_t orderedMapSGetCount(OrderedMapS *map);

#endif // C_UNORDERED_MAP_H

#ifndef C_ORDERED_MAP_H
#define C_ORDERED_MAP_H

#include <stdlib.h>

#ifndef C_ORDERED_MAP_INTERNAL
typedef void OrderedMapS;
#endif // C_ORDERED_MAP_INTERNAL

OrderedMapS *OrderedMapSCreate();
void OrderedMapSDestroy(OrderedMapS *map);
void OrderedMapSInsert(OrderedMapS *map, const char *key, void *value, size_t sizeof_value);
void OrderedMapSInsertOrReplace(OrderedMapS *map, const char *key, void *value);
void OrderedMapSErase(OrderedMapS *map);
void OrderedMapSClear(OrderedMapS *map);

int OrderedMapSAtKeyI(OrderedMapS *map, const char *key);
int OrderedMapSAtIndexI(OrderedMapS *map, size_t i);

const char *OrderedMapSKeyAtIndex(OrderedMapS *map, size_t i);

size_t OrderedMapSGetCount(OrderedMapS *map);

#endif // C_UNORDERED_MAP_H

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "../types.h"

typedef const char* cstring;

typedef struct
{
	string k;
	u32 v;
} KValue;

typedef struct
{
	KValue* t;
	u32 size;
	u32 n_elements;
} HashTable;

HashTable* HashTable_new(u32 size);
void HashTable_delete(HashTable* ht);
u32 HashTable_find(HashTable* ht, cstring key);

#endif

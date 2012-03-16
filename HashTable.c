#include "HashTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
**************************************************************************
*                                                                        *
*          General Purpose Hash Function Algorithms Library              *
*                                                                        *
* Author: Arash Partow - 2002                                            *
* URL: http://www.partow.net                                             *
* URL: http://www.partow.net/programming/hashfunctions/index.html        *
*                                                                        *
* Copyright notice:                                                      *
* Free use of the General Purpose Hash Function Algorithms Library is    *
* permitted under the guidelines and in accordance with the most current *
* version of the Common Public License.                                  *
* http://www.opensource.org/licenses/cpl1.0.php                          *
*                                                                        *
**************************************************************************
*/
static unsigned int HashFun(const char* str, unsigned int len)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}

HashTable* HashTable_new(u32 size)
{
	HashTable* ret = (HashTable*)malloc(sizeof(HashTable));
	u32 mem = sizeof(KValue) * size;
	ret->t = (KValue*)malloc(mem);
	ret->size = size;
	memset(ret->t, 0, mem);
	return ret;
}

void HashTable_delete(HashTable* ht)
{
	for (u32 i = 0; i < ht->size; i++)
		free(ht->t[i].k);
	free(ht->t);
	free(ht);
}

u32 HashTable_find(HashTable* ht, cstring name)
{
	u32 l = strlen(name);
	u32 cur = HashFun(name, l) % ht->size;
	while (ht->t[cur].k && strcmp(ht->t[cur].k, name))
		if (++cur >= ht->size)
			cur = 0;
	if (!ht->t[cur].k)
	{
		ht->t[cur].k = (string)malloc(l + 1);
		strcpy(ht->t[cur].k, name);
		ht->t[cur].v = ht->n_elements++;
	}
	return ht->t[cur].v;
}

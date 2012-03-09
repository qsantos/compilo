#include <stdio.h>

#include "HashTable.h"

int main()
{
	HashTable* ht = HashTable_new(256);
	printf("%lu\n", HashTable_find(ht, "hibou"));
	printf("%lu\n", HashTable_find(ht, "hibor"));
	printf("%lu\n", HashTable_find(ht, "hibor"));
	printf("%lu\n", HashTable_find(ht, "hibou"));
	HashTable_delete(ht);
	return 0;
}

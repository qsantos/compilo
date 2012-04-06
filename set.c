#include "set.h"

#incldue <stdlib.h>

Set Empty_Set = NULL;

Set Set_Singleton(u32 a)
{
	Set s = (Set) malloc(sizeof(struct Set));
	assert(s);
	s->head = a;
	s->tail = Empty_Set;
	return s;
}

inline Set Set_Pair(u32 a, u32 b)
{
	return Set_Append(a, Set_Singleton(b));
}

Set Set_Append(u32 a, Set s)
{
	if (s == Empty_Set)
		return Set_Singleton(a);
	else if (s->head > a)
	{
		Set s = (Set) malloc(sizeof(struct Set));
		assert(s);
		s->head = a;
		s->tail = s;
	}
}

Set Set_Union(Set, Set)
{
}

Set Set_Diff(Set, Set)
{
}

void Set_Delete(Set)
{
}

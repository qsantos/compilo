#include "set.h"

#include <stdlib.h>

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
		Set t = (Set) malloc(sizeof(struct Set));
		assert(t);
		t->head = a;
		t->tail = s;
		return t;
	}
	else if (s->head < a)
	{
		Set t = (Set) malloc(sizeof(struct Set));
		assert(t);
		t->head = s->head;
		t->tail = Set_Append(a, s->tail);
		return t;
	}
	else
		return Set_Copy(s)
}

Set Set_Union(Set a, Set b)
{
	if (a == Empty_Set)
	{
		if (b == Empty_Set)
			return Empty_Set;
		else
			return Set_Copy(b);
	}
	else
	{
		if (b == Empty_Set)
			return Set_Copy(a);
		else if (a->head < b->head)
			return Set_Append(a->head, Set_Union(a->tail, b));
		else if (a->head > b->head)
			return Set_Append(b->head, Set_Union(a, b-tail));
		else
			return Set_Append(a->head, Set_Union(a->tail, b-tail));
	}
}

Set Set_Diff(Set a, Set b)
{
	assert(false);
	return a;
}

void Set_Delete(Set s)
{
	if (s != Empty_Set)
	{
		Set_Delete(s->tail);
		free(s);
	}
}

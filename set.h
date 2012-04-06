#ifndef SET_H
#define SET_H

#include "types.h"

typedef struct Set* Set;
struct Set
{
	u32 head;
	Set tail;
};

extern Set Empty_Set;

Set  Set_Pair  (u32, u32);
Set  Set_Append(u32, Set);
Set  Set_Union (Set, Set);
Set  Set_Diff  (Set, Set);
void Set_Delete(Set);

#endif

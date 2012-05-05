#ifndef INTGRAPH_H
#define INTGRAPH_H

#include "flow.h"

typedef struct
{
	bool interf;
	bool pref;
} edge;

typedef struct
{
	u32   n;
	u32*  d;
	bool* move;
	bool* dead;
	edge* e;
} IntGraph;

IntGraph* IntGraph_New      (u32);
void      IntGraph_Delete   (IntGraph*);

bool      IntGraph_AddInterf(IntGraph*, u32, u32);
bool      IntGraph_DelInterf(IntGraph*, u32, u32);
bool      IntGraph_AddMove  (IntGraph*, u32, u32);
bool      IntGraph_DelMove  (IntGraph*, u32, u32);

void      IntGraph_Simplify (IntGraph*, u32);
void      IntGraph_Coalesce (IntGraph*, u32, u32);

IntGraph* IntGraph_FromFlow(Flow*);

#endif

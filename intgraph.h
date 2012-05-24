/*\
 *       \\          A Tiny C to SPIM Compiler                     //
 *        \\_        Copyright (C) 2012 Thomas  GREGOIRE         _//
 *     .---(')                          Quentin SANTOS          (')---. 
 *   o( )_-\_        Logos by jgs                                _/-_( )o
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

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
	u32*  move;
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

IntGraph* IntGraph_FromFlow (Flow*);

#endif

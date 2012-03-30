/*
 *   A Tiny C to SPIM Compiler
 *   Copyright (C) 2012 Thomas GREGOIRE, Quentin SANTOS
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
 */

#ifndef ASM_H
#define ASM_H

#include "types.h"
#include "ast.h"
#include "context.h"

typedef enum
{
	INSN_SET,  INSN_MOV,
	INSN_NEG,  INSN_AND,  INSN_OR,  INSN_XOR,
	INSN_NOT,  INSN_LAND, INSN_LOR,
	INSN_EQ,   INSN_NEQ,  INSN_LE,  INSN_LT,  INSN_GE,  INSN_GT,
	INSN_ADD,  INSN_SUB,  INSN_MUL, INSN_DIV, INSN_MOD,
	INSN_PUSH, INSN_POP,
	INSN_JMP,  INSN_JZ,   INSN_JNZ, INSN_CALL,
} ASM_INSN;

typedef struct RegList RegList;
struct RegList
{
	u32      head;
	RegList* tail;
};

typedef struct
{
	ASM_INSN insn;
	union
	{
		struct { u32 r0; u32 r1; u32 r2; } r;
		RegList p;
	} v;
} Instr;

typedef struct
{
	Instr* code;
	u32 len;
	u32 avail;
	u32 reg;
} ASM;

ASM* ASM_New();
void ASM_Delete (ASM*);
void ASM_Push   (ASM*, ASM_INSN, u32, u32, u32);
u32  ASM_NewReg (ASM*);
u32  ASM_GenExpr(ASM*, Expr*, Context*);

#endif

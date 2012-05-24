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

#ifndef ASM_H
#define ASM_H

#include "types.h"
#include "ast.h"
#include "context.h"
#include "set.h"

typedef enum
{
	INSN_STOP,
	INSN_SET,  INSN_MOV,  INSN_MRD, INSN_MWR,  INSN_RGA,
	INSN_NOT,  INSN_AND,  INSN_OR,  INSN_XOR,
	INSN_LNOT, INSN_LAND, INSN_LOR,
	INSN_EQ,   INSN_NEQ,  INSN_LE,  INSN_LT,   INSN_GE,  INSN_GT,
	INSN_ADD,  INSN_SUB,  INSN_MUL, INSN_DIV,  INSN_MOD,
	INSN_JMP,  INSN_JZ,   INSN_JNZ, INSN_CALL, INSN_RET,
	INSN_LBL,
} ASM_INSN;

typedef struct
{
	ASM_INSN insn;
	union
	{
		struct { u32 r0; u32 r1; u32 r2; } r;
		u32stack* p;
	} v;
} Instr;

typedef struct
{
	Instr*      code;
	u32         n_code;
	u32         a_code;
	
	u32         n_regs;
	
	u32*        labels;
	u32         n_labels;
	u32         a_labels;
	
	u32stack**  funCalls; // for each function, list of calls
	u32stack*   spilled;
} ASM;

ASM* ASM_New       (Context*);
void ASM_Delete    (ASM*);
void ASM_Push      (ASM*, ASM_INSN, u32, u32, u32);
void ASM_PushList  (ASM*, ASM_INSN, u32stack*);
u32  ASM_NewReg    (ASM*, Context*);
u32  ASM_NewLabel  (ASM*);
void ASM_LabelPos  (ASM*, u32);
u32  ASM_GenExpr   (ASM*, Context*, Expr*);
void ASM_GenStmt   (ASM*, Context*, Stmt*);
void ASM_GenFun    (ASM*, Context*, FunDecl*);
void ASM_GenProgram(ASM*, Context*, Program*);

#endif

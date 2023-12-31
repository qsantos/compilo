%{
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

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "error.h"

extern Program* current_prog;
extern int yylex(void);
extern int yyerror(const char*);
%}

%union
{
  long       integer;
  char*      symbol;

  LValue*    lvalue;
  Expr*      expr;
  ExprList*  exprlist;
  Type*      type;
  Stmt*      stmt;
  StmtList*  stmtlist;
  Param*     param;
  ParamList* paramlist;
  FunDecl*   fundecl;
  Program*   program;
};

%token <integer> INTEGER
%token <symbol> SYMBOL
%token WHILE DO FOR IF ELSE RETURN
%token VOID CHAR INT
%token LAND LOR

%nonassoc IF_ALONE
%nonassoc ELSE
%left LAND LOR
%left '='
%left EQ NEQ LE GE '<' '>'
%left '?' ':'
%left '!' '~' '&' '|' '^'
%left '+' '-'
%left '*' '/' '%'
%nonassoc MINUS_ALONE
%nonassoc LSTAR
%nonassoc RSTAR
%nonassoc ESP_ALONE
%left INC DEC
%left EQADD EQSUB EQMUL EQDIV EQMOD EQAND EQOR EQXOR

%type <stmt> instr
%type <stmt> statement
%type <stmtlist> statement_list
%type <lvalue> lvalue
%type <expr> expression
%type <exprlist> expr_list
%type <type> type
%type <fundecl> fun_declaration
%type <program> program
%type <param> parameter
%type <paramlist> param_list
%type <paramlist> params

%start start

%locations
%error-verbose

%%

start: program                                             { current_prog = $1;                                 }
;

program:
                                                           { $$ = NULL;                                         }
     | fun_declaration program                             { $$ = Program_New($1, $2);                          }
;

fun_declaration: type SYMBOL '(' params ')' statement      { $$ = FunDecl_New($1, $2, $4, $6, (position*) &@$); }
;

type:
       VOID                                                { $$ = Type_Void();                                  }
     | CHAR                                                { $$ = Type_Char();                                  }
     | INT                                                 { $$ = Type_Int();                                   }
     | type '*'                                            { $$ = Type_Ptr($1);                                 }
;

instr:
                                                           { $$ = Stmt_Nothing();                               }
     | type SYMBOL                                         { $$ = Stmt_Decl($1, $2, (position*) &@$);           }
     | expression                                          { $$ = Stmt_Expr($1);                                }

statement:
       instr ';'                                           { $$ = $1;                                           }
     | WHILE '(' expression ')' statement                  { $$ = Stmt_While($3, $5);                           }
     | DO statement WHILE '(' expression ')'               { $$ = Stmt_Do($2, $5);                              }
     | FOR '(' instr ';' expression ';' instr ')' statement{ $$ = Stmt_For($3, $5, $7, $9);                     }
     | IF '(' expression ')' statement %prec IF_ALONE      { $$ = Stmt_If($3, $5, NULL);                        }
     | IF '(' expression ')' statement ELSE statement      { $$ = Stmt_If($3, $5, $7);                          }
     | '{' statement_list '}'                              { $$ = Stmt_Block($2);                               }
     | RETURN expression ';'                               { $$ = Stmt_Return($2);                              }
;

statement_list:
       statement statement_list                            { $$ = StmtList_New($1, $2);                         }
     |                                                     { $$ = NULL;                                         }
     | type SYMBOL '=' expression ';' statement_list       { $$ = StmtList_New(Stmt_Decl($1, $2, (position*) &@$), StmtList_New(Stmt_Expr(Expr_Aff(LValue_Var($2, (position*) &@$), $4, (position*) &@$)), $6)); }
;

parameter: type SYMBOL                                     { $$ = Param_New($1, $2, (position*) &@$);           }
;

param_list:
       parameter                                           { $$ = ParamList_New($1, NULL);                      }
     | parameter ',' param_list                            { $$ = ParamList_New($1, $3);                        }
;

params:
       VOID                                                { $$ = ParamList_Void();                             }
     | param_list                                          { $$ = $1;                                           }

lvalue:
       SYMBOL                                              { $$ = LValue_Var($1, (position*) &@$);              }
     | '$' SYMBOL                                          { $$ = LValue_Ref(Expr_Var($2, (position*) &@$), (position*) &@$);              }
     | SYMBOL '[' expression ']'                           { $$ = LValue_Ref(Expr_Add(Expr_Var($1, (position*) &@$), Expr_Mul($3, Expr_Integer(4, (position*) &@$), (position*) &@$), (position*) &@$), (position*) &@$); }
;

expression:
       INTEGER                                             { $$ = Expr_Integer ($1,       (position*) &@$);     }
     | SYMBOL '(' expr_list ')'                            { $$ = Expr_Fun_Call($1, $3,   (position*) &@$);     }
     | SYMBOL '(' ')'                                      { $$ = Expr_Fun_Call($1, NULL, (position*) &@$);     }
     | lvalue '=' expression                               { $$ = Expr_Aff     ($1, $3,   (position*) &@$);     }
     | SYMBOL '[' expression ']'                           { $$ = Expr_Deref(Expr_Add(Expr_Var($1, (position*) &@$), Expr_Mul($3, Expr_Integer(4, (position*) &@$), (position*) &@$), (position*) &@$), (position*) &@$);     }
     | SYMBOL                                              { $$ = Expr_Var     ($1,       (position*) &@$);     }
     | '!' expression                                      { $$ = Expr_Not     ($2,       (position*) &@$);     }
     | '~' expression                                      { $$ = Expr_Lnot    ($2,       (position*) &@$);     }
     | expression '&'  expression                          { $$ = Expr_And     ($1, $3,   (position*) &@$);     }
     | expression '|'  expression                          { $$ = Expr_Or      ($1, $3,   (position*) &@$);     }
     | expression '^'  expression                          { $$ = Expr_Xor     ($1, $3,   (position*) &@$);     }
     | expression LAND expression                          { $$ = Expr_Land    ($1, $3,   (position*) &@$);     }
     | expression LOR  expression                          { $$ = Expr_Lor     ($1, $3,   (position*) &@$);     }
     | expression EQ   expression                          { $$ = Expr_Eq      ($1, $3,   (position*) &@$);     }
     | expression NEQ  expression                          { $$ = Expr_Neq     ($1, $3,   (position*) &@$);     }
     | expression LE   expression                          { $$ = Expr_Le      ($1, $3,   (position*) &@$);     }
     | expression '<'  expression                          { $$ = Expr_Lt      ($1, $3,   (position*) &@$);     }
     | expression GE   expression                          { $$ = Expr_Ge      ($1, $3,   (position*) &@$);     }
     | expression '>'  expression                          { $$ = Expr_Gt      ($1, $3,   (position*) &@$);     }
     | expression '+'  expression                          { $$ = Expr_Add     ($1, $3,   (position*) &@$);     }
     | expression '-'  expression                          { $$ = Expr_Sub     ($1, $3,   (position*) &@$);     }
     | expression '*'  expression                          { $$ = Expr_Mul     ($1, $3,   (position*) &@$);     }
     | expression '/'  expression                          { $$ = Expr_Div     ($1, $3,   (position*) &@$);     }
     | expression '%'  expression                          { $$ = Expr_Mod     ($1, $3,   (position*) &@$);     }
     | '-' expression %prec MINUS_ALONE                    { $$ = Expr_Minus   ($2,       (position*) &@$);     }
     | '*' expression %prec RSTAR                          { $$ = Expr_Deref   ($2,       (position*) &@$);     }
     | '&' SYMBOL     %prec ESP_ALONE                      { $$ = Expr_Addr    ($2,       (position*) &@$);     }
     | expression '?' expression ':' expression            { $$ = Expr_Ifte    ($1,$3,$5, (position*) &@$);     }
     | '(' expression ')'                                  { $$ = $2;                                           }


     | SYMBOL INC              { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Add(Expr_Var($1, (position*) &@$), Expr_Integer(1, (position*) &@$), (position*) &@$), (position*) &@$); }
     | SYMBOL DEC              { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Sub(Expr_Var($1, (position*) &@$), Expr_Integer(1, (position*) &@$), (position*) &@$), (position*) &@$); }
     | SYMBOL EQADD expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Add(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQSUB expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Sub(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQMUL expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Mul(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQDIV expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Div(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQMOD expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Mod(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQAND expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_And(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQOR  expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Or (Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
     | SYMBOL EQXOR expression { $$ = Expr_Aff(LValue_Var($1, (position*) &@$), Expr_Xor(Expr_Var($1, (position*) &@$), $3, (position*) &@$), (position*) &@$); }
;

expr_list:
       expression ',' expr_list                            { $$ = ExprList_New($1, $3);                         }
     | expression                                          { $$ = ExprList_New($1, NULL);                       }
;

%%

int yyerror(const char* err)
{
  fprintf(stderr, "Line %lu, character %lu: %s\n", row, col, err);
  return 0;
}

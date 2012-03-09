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
#include "global.h"

extern int yylex(void);
extern int yyerror(const char*);
%}

%union
{
  long       integer;
  char*      symbol;

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

%nonassoc IF_ALONE
%nonassoc ELSE
%left '='
%left EQ NEQ LE GE '<' '>'
%left '?' ':'
%left '!'
%left '%'
%left '+' '-'
%left '*' '/'
%nonassoc MINUS_ALONE

%type <stmt> instr
%type <stmt> statement
%type <stmtlist> statement_list
%type <expr> expression
%type <exprlist> expr_list
%type <type> type
%type <fundecl> fun_declaration
%type <program> program
%type <param> parameter
%type <paramlist> param_list
%type <paramlist> params

%start start

%error-verbose

%%

start: program                                             { current_prog = $1;                     }
;

program:
       fun_declaration program                             { $$ = Program_New($1, $2);              }
     |                                                     { $$ = NULL;                             }
;

fun_declaration: type SYMBOL '(' params ')' statement      { $$ = FunDecl_New($1, $2, $4, $6);      }
;

type:
       VOID                                                { $$ = Type_Void();                      }
     | CHAR                                                { $$ = Type_Char();                      }
     | INT                                                 { $$ = Type_Int();                       }
     | type '*'                                            { $$ = Type_Ptr($1);                     }
;

instr:
                                                           { $$ = Stmt_Nothing();                   }
     | type SYMBOL '=' expression                          { $$ = Stmt_Decl($1, $2, $4);            }
     | type SYMBOL                                         { $$ = Stmt_Decl($1, $2, NULL);          }
     | expression                                          { $$ = Stmt_Expr($1);                    }

statement:
       instr ';'                                           { $$ = $1;                               }
     | WHILE '(' expression ')' statement                  { $$ = Stmt_While($3, $5);               }
     | DO statement WHILE '(' expression ')'               { $$ = Stmt_Do($2, $5);                  }
     | FOR '(' instr ';' instr ';' instr ')' statement     { $$ = Stmt_For($3, $5, $7, $9);         }
     | IF '(' expression ')' statement %prec IF_ALONE      { $$ = Stmt_If($3, $5, NULL);            }
     | IF '(' expression ')' statement ELSE statement      { $$ = Stmt_If($3, $5, $7);              }
     | '{' statement_list '}'                              { $$ = Stmt_Block($2);                   }
     | RETURN expression ';'                               { $$ = Stmt_Return($2);                  }
;

statement_list:
       statement statement_list                            { $$ = StmtList_New($1, $2);             }
     |                                                     { $$ = NULL;                             }
;

parameter: type SYMBOL                                     { $$ = Param_New($1, $2);                }
;

param_list:
       parameter                                           { $$ = ParamList_New($1, NULL);          }
     | parameter ',' param_list                            { $$ = ParamList_New($1, $3);            }
;

params:
       VOID                                                { $$ = ParamList_Void();                 }
     | param_list                                          { $$ = $1;                               }

expression:
       INTEGER                                             { $$ = Expr_Integer($1);                 }
     | SYMBOL '(' expr_list ')'                            { $$ = Expr_Fun_Call($1, $3);            }
     | SYMBOL '(' ')'                                      { $$ = Expr_Fun_Call($1, NULL);          }
     | SYMBOL '=' expression                               { $$ = Expr_Aff($1, $3);                 }
     | SYMBOL                                              { $$ = Expr_Var($1);                     }
     | '!' expression                                      { $$ = Expr_Neg($2);                     }
     | expression EQ  expression                           { $$ = Expr_Eq($1, $3);                  }
     | expression NEQ expression                           { $$ = Expr_Neq($1, $3);                 }
     | expression LE  expression                           { $$ = Expr_Le($1, $3);                  }
     | expression '<' expression                           { $$ = Expr_Lt($1, $3);                  }
     | expression GE  expression                           { $$ = Expr_Ge($1, $3);                  }
     | expression '>' expression                           { $$ = Expr_Gt($1, $3);                  }
     | expression '+' expression                           { $$ = Expr_Add($1, $3);                 }
     | expression '-' expression                           { $$ = Expr_Sub($1, $3);                 }
     | expression '*' expression                           { $$ = Expr_Mul($1, $3);                 }
     | expression '/' expression                           { $$ = Expr_Div($1, $3);                 }
     | expression '%' expression                           { $$ = Expr_Modulo($1, $3);              }
     | '-' expression %prec MINUS_ALONE                    { $$ = Expr_Minus($2);                   }
     | expression '?' expression ':' expression            { $$ = Expr_Ifte($1,$3,$5);              }
     | '(' expression ')'                                  { $$ = $2;                               }
;

expr_list:
       expression ',' expr_list                            { $$ = ExprList_New($1, $3);             }
     | expression                                          { $$ = ExprList_New($1, NULL);           }
;

%%

int yyerror(const char* err)
{
  fprintf(stderr, "Line %lu, character %lu: %s\n", row, col, err);
  return 0;
}

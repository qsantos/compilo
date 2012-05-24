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

#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "ast.h"
#include "parser.h"
#include "error.h"

#define YY_USER_ACTION \
{\
	yylloc.first_line = row; \
	yylloc.first_column = col; \
	col += yyleng; \
	yylloc.last_column = col; \
	yylloc.last_line = row; \
}

int yyerror(const char*);

static inline string String_Copy(string s)
{
  string c = (char*) malloc(strlen(s) + 1);
  assert(c);
  strcpy(c, s);
  return c;
}
%}

SYMBOL          [_a-zA-Z][_a-zA-Z0-9]*
INTEGER         [0-9]+

%x COMMENT

%%

"/*"                          { BEGIN(COMMENT);               }
<COMMENT>"*/"                 { BEGIN(INITIAL);               }
<COMMENT>.                    {                               }

"while"                       { Char_Move(5); return WHILE;   }
"do"                          { Char_Move(2); return DO;      }
"for"                         { Char_Move(3); return FOR;     }
"if"                          { Char_Move(2); return IF;      }
"else"                        { Char_Move(4); return ELSE;    }
"return"                      { Char_Move(6); return RETURN;  }

"void"                        { Char_Move(4); return VOID;    }
"char"                        { Char_Move(4); return CHAR;    }
"int"                         { Char_Move(3); return INT;     }

"&&"                          { Char_Move(2); return LAND;    }
"||"                          { Char_Move(2); return LOR;     }
"=="                          { Char_Move(2); return EQ ;     }
"!="                          { Char_Move(2); return NEQ;     }
"<="                          { Char_Move(2); return LE ;     }
">="                          { Char_Move(2); return GE ;     }

[-(){},;+*=/%<>!?:~&|\^$\[\]] { Char_Move(1); return *yytext; }

{INTEGER}                     { Char_Move(strlen(yytext)); yylval.integer = atol(yytext);       return INTEGER; }
{SYMBOL}                      { Char_Move(strlen(yytext)); yylval.symbol = String_Copy(yytext); return SYMBOL;  }

[ \r\t]+                      { Char_Move(strlen(yytext));    }
\n                            { Char_Newline();               }
.                             { yyerror("Unknown character"); }

%%

int yywrap(void)
{
  return 1;
}

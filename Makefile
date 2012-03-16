CFLAGS := -O3 -Wall -Wextra -ansi -pedantic -std=c99
CFILES := ast.c main.c printer.c error.c latex.c static.c hash.c u32stack.c
OFILES := $(CFILES:.c=.o)

.PHONY: all clean

all: compilo

compilo: parser.tab.o lex.yy.o $(OFILES)
	gcc $^ -o $@

lex.yy.o: lex.yy.c
	gcc $(CFLAGS) -Wno-unused-function -Wno-implicit-function-declaration -c $^

%.o: %.c
	gcc $(CFLAGS) -c $^

lex.yy.c: lexer.lex
	flex --header-file=lexer.h $^

parser.tab.c: parser.y
	bison --defines=parser.h $^

clean:
	-rm -f $(OFILES) parser.tab.c lex.yy.c parser.tab.o lex.yy.o lexer.h parser.h

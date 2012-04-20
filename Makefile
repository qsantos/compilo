CFLAGS := -O3 -Wall -Wextra -ansi -pedantic -std=c99 -g
CFILES := asm.c ast.c context.c error.c exec.c hash.c latex.c main.c printer.c regalloc.c set.c salmon.c static.c u32stack.c
OFILES := $(CFILES:.c=.o)

.PHONY: all clean

all: compilo

test: compilo
	./test

ltest: compilo
	./ltest

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

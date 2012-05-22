CFLAGS := -Wall -Wextra -ansi -pedantic -std=c99 -g #-O3
CFILES := asm.c ast.c context.c error.c exec.c flow.c hash.c intgraph.c latex.c main.c printer.c regalloc.c set.c static.c u32stack.c
OFILES := $(CFILES:.c=.o)
TARGET := compilo

.PHONY: all clean

all: $(TARGET)

$(TARGET): parser.tab.o lex.yy.o $(OFILES)
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
	rm -f $(OFILES) parser.tab.c lex.yy.c parser.tab.o lex.yy.o lexer.h parser.h

test: $(TARGET)
	@./test

ltest: $(TARGET)
	@./ltest

tgz: $(TARGET)
	tar zcvf GregoireSantos.tgz *.h *.c lexer.lex parser.y examples CHANGELOG $(TARGET) LICENCE LOGO ltest Makefile README test

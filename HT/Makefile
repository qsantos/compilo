CFLAGS  = -Wall -Werror -pedantic -std=c99 -O3
LDFLAGS = -O3
EXEC    = hashtest
SRC     = $(wildcard *.c)
OBJ     = $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	gcc $(LDFLAGS) $(OBJ) -o $(EXEC)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	@if [ -e $(EXEC) ]; then rm $(EXEC); fi
	@if [ -n "`find -name \*.o`" ]; then rm *.o; fi

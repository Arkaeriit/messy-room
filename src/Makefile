CFLAGS += -g -Wall -Wextra -Werror
CC ?= gcc

SRC := messy-room.c test.c
HEADER := messy-room.h

OBJS := $(patsubst %.c,%.o,$(SRC))

all: test libmessy-room.a

test: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

libmessy-room.a: messy-room.o
	ar rcs $@ $^

%.o: %.c $(HEADER)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(OBJS)
	rm -rf test
	rm -rf libmessy-room.a
	rm -rf test1.mr
	rm -rf test2.mr


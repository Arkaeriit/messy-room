CFLAGS += -g -Wall -Wextra -Werror
CC ?= gcc

SRC := mr_heaplet.c test.c
HEADER := mr_heaplet.h

OBJS := $(patsubst %.c,%.o,$(SRC))

test: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(OBJS)
	rm -rf test1.mr
	rm -rf test2.mr

CFLAGS := -g -Wall -Wextra -Werror
CC := gcc
SRC := mr_heaplet.c
HEADER := mr_heaplet.h

OBJS := $(patsubst %.c,%.o,$(SRC))

compile: $(OBJS) $(HEADER)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(OBJS)

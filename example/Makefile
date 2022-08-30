CFLAGS += -I../src/ -g -Wall -Wextra -L../src/
CC ?= gcc

SRC := main.c kv-over-messy-room.c
HEADER := ../src/messy-room.h kv-over-messy-room.h

OBJS := $(patsubst %.c,%.o,$(SRC))

all: messy-kv

messy-kv: $(OBJS) ../src/libmessy-room.a
	$(CC) $(OBJS) $(CFLAGS) -lmessy-room -o $@

../src/libmessy-room.a: ../src/messy-room.c ../src/messy-room.h
	cd ../src; \
	make libmessy-room.a

%.o: %.c $(HEADER)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf *.o
	rm -rf messy-kv


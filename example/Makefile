CFLAGS += -I../src/ -g -Wall -Wextra -L../src/
CC ?= gcc
INSTALL_PATH_BIN ?= /usr/local/bin

SRC := main.c kv-over-messy-room.c
HEADER := ../src/messy-room.h kv-over-messy-room.h

OBJS := $(patsubst %.c,%.o,$(SRC))

all: messy-kv

messy-kv: $(OBJS) ../src/libmessy-room.a
	$(CC) $(OBJS) $(CFLAGS) -lmessy-room -o $@

../src/libmessy-room.a: ../src/messy-room.c ../src/messy-room.h
	cd ../src; \
	make libmessy-room.a

install: messy-kv
	mkdir -p $(INSTALL_PATH_BIN)
	cp -f messy-kv $(INSTALL_PATH_BIN)/messy-kv

uninstall:
	rm -f $(INSTALL_PATH_BIN)/messy-kv
	mkdir -p $(INSTALL_PATH_BIN)
	rmdir --ignore-fail-on-non-empty $(INSTALL_PATH_BIN)

%.o: %.c $(HEADER)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf *.o
	rm -rf messy-kv


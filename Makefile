CC = gcc
CFLAGS = -I include -I external
DEV_CFLAGS = -g -Wall
PRO_CFLAGS = -O2

SRC = $(wildcard src/*.c) $(wildcard external/*.c)

.PHONEY: build

dev: $(SRC)
	$(CC) $^ main.c $(CFLAGS) -g  -o bin/main

build: $(SRC)
	$(CC) $^ main.c $(CFLAGS) $(PRO_CFLAGS) -o bin/main

run: dev
	./bin/main

test_move_generation: $(SRC)
	$(CC) tests/test_move_generation.c $^ $(CFLAGS) $(DEV_CFLAGS) -o bin/test_move_generation
	./bin/test_move_generation
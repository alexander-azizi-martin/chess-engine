CC = gcc
CFLAGS = -I include
DEV_CFLAGS = -g -Wall -Werror
PRO_CFLAGS = -O2 -fwhole-program

SRC = $(wildcard src/*.c)

.PHONEY: build

dev: $(SRC)
	$(CC) $^ main.c $(CFLAGS) $(DEV_CFLAGS) -o main.exe

build: $(SRC)
	$(CC) $^ main.c $(CFLAGS) $(PRO_CFLAGS) -o main.exe

run: dev
	main.exe
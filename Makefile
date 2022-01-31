CC=gcc
CFLAGS=-g -Wall -I include

BIN=bin
INC=include

SRC=src
SRCS=$(wildcard $(SRC)/*.c)

OBJ=obj
OBJS=$(patsubst $(SRC)/%.c,$(OBJ)/%.o, $(SRCS))

TEST=tests
TESTS=$(wildcard $(TEST)/*.c)
TESTBINS=$(patsubst $(TEST)/%.c,$(TEST)/$(BIN)/%, $(TESTS))

run: $(BIN)/main
	$<

build: $(BIN)/main

test: $(TESTBINS)
	for test in $(TESTBINS); do $$test --ascii; done

release: CFLAGS=-Wall -O2 -DNDEBUG -I include
release: clean
release: $(BIN)/main

$(BIN)/main: $(OBJS) main.c
	$(CC) $(CFLAGS) $(OBJS) main.c -o $(BIN)/main

$(OBJ)/%.o: $(SRC)/%.c $(INC)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST)/$(BIN)/%: $(TEST)/%.c $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $< -o $@ -lcriterion 

$(OBJ):
	mkdir $@

$(TEST)/$(BIN):
	mkdir $@

clean:
	$(RM) -r $(OBJ)/*
	$(RM) -r $(TEST)/bin/*
	$(RM) -r bin/*
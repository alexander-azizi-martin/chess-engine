#ifndef TRANSPOTION_TABLE_H
#define TRANSPOTION_TABLE_H

#include "defs.h"
#include "move.h"

typedef struct 
{
    U64 key;
    Move move;
    bool deleted;
    bool occupied;
} Entry;

typedef struct 
{
    Entry *entries;
    int num_entries;
    int size;
} TranspositionTable;

/**
 * Returns a transposition table of the given size that was allocated on the heap. Null will be
 * returned if the table was not able to be allocated on the heap.
 **/
TranspositionTable* table_init(int size);


/**
 * Frees the memory the transposition table was taking up.
 **/
void table_free(TranspositionTable* table);


/**
 * Inserts an entry into the transposition table. Returns whether it was able
 * to insert the element.
 **/
bool table_insert(TranspositionTable* table, U64 key, Move move);

/**
 * Deletes the entry with the given key from the transposition table. Returns 
 * whether it was able to delete the element.
 **/
bool table_delete(TranspositionTable* table, U64 key);

/**
 * Returns the move corresponding to the given key in the transposition table.
 **/
Move* table_lookup(TranspositionTable* table, U64 key);

#endif
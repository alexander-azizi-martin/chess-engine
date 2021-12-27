#include <stdlib.h>
#include <stdbool.h>
#include "transposition_table.h"

/**
 * Returns a transposition table of the given size that was allocated on the heap. Null will be
 * returned if the table was not able to be allocated on the heap.
 **/
TranspositionTable* table_init(int size)
{
    TranspositionTable* table = malloc(sizeof(TranspositionTable));

    if (table == NULL)
        return NULL;

    table->entries = calloc(size, sizeof(Entry));
    table->num_entries = 0;
    table->size = size;

    if (table->entries == NULL)
        return NULL;

    return table;
}

/**
 * Frees the memory the transposition table was taking up.
 **/
void table_free(TranspositionTable* table)
{
    if (table == NULL)
        return;
    
    free(table->entries);
    free(table);
}

/**
 * Inserts an entry into the transposition table. Returns whether it was able
 * to insert the element.
 **/
bool table_insert(TranspositionTable* table, U64 key, Move move)
{
    if (table->num_entries == table->size)
        return false;

    // Probs the table to find an empty slot
    int index = key % table->size;
    while (table->entries[index].occupied == true)
    {
        index ++;

        // Wraps around to the beginning of the table
        if (index >= table->size)
            index = 0;
    }

    table->entries[index].occupied = true;
    table->entries[index].deleted = false;
    table->entries[index].key = key;
    table->entries[index].move = move;
    table->num_entries++;

    return true;
}

/**
 * Deletes the entry with the given key from the transposition table. Returns 
 * whether it was able to delete the element.
 **/
bool table_delete(TranspositionTable* table, U64 key)
{
    // Probs the table until an empty slot is found or a slot with the given key
    int index = key % table->size;
    while (table->entries[index].occupied == true || table->entries[index].deleted == true)
    {   
        if (table->entries[index].key == key)
            break;

        index ++;

        // Wraps around to the beginning of the table
        if (index >= table->size)
            index = 0;
    }

    if (table->entries[index].key != key)
        return false;
    
    table->entries[index].deleted = true;
    table->entries[index].occupied = false;
    table->num_entries--;
    return true;
}

/**
 * Returns the move corresponding to the given key in the transposition table.
 **/
Move* table_lookup(TranspositionTable* table, U64 key)
{
    // Probs the table until an empty slot is found or a slot with the given key
    int index = key % table->size;
    while (table->entries[index].occupied == true || table->entries[index].deleted == true)
    {   
        if (table->entries[index].key == key)
            break;

        index ++;

        // Wraps around to the beginning of the table
        if (index >= table->size)
            index = 0;
    }

    return (table->entries[index].key == key && table->entries[index].deleted == false) 
        ? &(table->entries[index].move) 
        : NULL;
}

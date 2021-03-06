#ifndef BITBOARD_H
#define BITBOARD_H

/*
Representation of a chess bitboard:

8 | 56  57  58  59  60  61  62  63
7 | 48  49  50  51  52  53  54  55
6 | 40  41  42  43  44  45  46  47
5 | 32  33  34  35  36  37  38  39
4 | 24  25  26  27  28  29  30  31
3 | 16  17  18  19  20  21  22  23
2 | 8   9   10  11  12  13  14  15
1 | 0   1   2   3   4   5   6   7
  --------------------------------
    A   B   C   D   E   F   G   H
*/

#include "defs.h"

typedef U64 BitBoard;

/** 
 * Returns the index (0-63) of the LSB in the given BitBoard.  
 **/
int bitboard_scan_forward(BitBoard board);

/**
 * Returns the index of the LSB from the given bitboard and 
 * sets it to 0. Returns -1 if the BitBoard does not have 
 * any non-zero bits
 **/
int bitboard_pop(BitBoard *board);

/**
 * Returns the number of non-zero bits in the given BitBoard.
 **/
int bitboard_count(BitBoard board);

/**
 * Prints the given BitBoard as a chess board.
 **/
void bitboard_print(BitBoard board);

#endif
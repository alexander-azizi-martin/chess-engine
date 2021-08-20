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

typedef U64 Bitboard;

typedef struct 
{
  U8 indices[64];
  U8 size;
} BitIndices;

int bitboard_scan_forward(Bitboard board);

void bitboard_index(BitIndices *bits, Bitboard board);

int bitboard_count(Bitboard board);

void bitboard_print(Bitboard board);

#endif
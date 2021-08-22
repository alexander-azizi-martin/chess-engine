#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "bitboard.h"

#define FileRankToSquare(f, r) (MASK_SQUARE[(r) * 8 + (f)])

Bitboard CLEAR_RANK[8];

Bitboard MASK_RANK[8];

Bitboard CLEAR_FILE[8];

Bitboard MASK_FILE[8];

Bitboard MASK_SQUARE[64];

#endif
#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "bitboard.h"

#define FileRankToSquare(f, r) (MASK_SQUARE[(r) * 8 + (f)])

const BitBoard CLEAR_RANK[8];

const BitBoard MASK_RANK[8];

const BitBoard CLEAR_FILE[8];

const BitBoard CLEAR_FILE_AB;

const BitBoard CLEAR_FILE_GH;

const BitBoard MASK_FILE[8];

const BitBoard MASK_SQUARE[64];

BitBoard MASK_PAWN_ATTACKS[2][64];

BitBoard MASK_KNIGHT_ATTACKS[64];

BitBoard MASK_KING_ATTACKS[64];

void init_lookup_tables();

#endif
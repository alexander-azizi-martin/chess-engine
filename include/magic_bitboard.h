#ifndef MAGIC_BITBOARD_H
#define MAGIC_BITBOARD_H

#include "bitboard.h"

typedef struct {
   BitBoard *attack_table;  // pointer to attack_table for each particular square
   BitBoard blocker_mask;  // to mask relevant squares of both lines (no outer squares)
   U64 magic_number; // magic 64-bit factor
   int shift;
} MagicInfo;

MagicInfo MAGIC_ROOK_TABLE[64];
MagicInfo MAGIC_BISHOP_TABLE[64];

BitBoard MASK_ROOK_ATTACKS[102400];
BitBoard MASK_BISHOP_ATTACKS[5248];

void magic_bitboards_init();

BitBoard lookup_bishop_attacks(int square, BitBoard occupied_squares); 

BitBoard lookup_rook_attacks(int square, BitBoard occupied_squares); 

#endif
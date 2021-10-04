#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

typedef uint8_t U8;
typedef uint32_t U32;
typedef uint64_t U64;

#define PieceColor(p) ((p & 0x8) != 0)

typedef enum
{
    WHITE,
    BLACK,
    WHITE_PAWNS,
    WHITE_ROOKS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_QUEENS,
    WHITE_KING,
    BLACK_PAWNS,
    BLACK_ROOKS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_QUEENS,
    BLACK_KING,
    EMPTY,
} Piece;

typedef enum
{
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
} Rank;

typedef enum
{
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
} File;

typedef enum
{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
} Square;

#endif
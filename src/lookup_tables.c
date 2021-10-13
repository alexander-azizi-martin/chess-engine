#include "lookup_tables.h"

const BitBoard CLEAR_RANK[8] = {
    0xffffffffffffff00,
    0xffffffffffff00ff,
    0xffffffffff00ffff,
    0xffffffff00ffffff,
    0xffffff00ffffffff,
    0xffff00ffffffffff,
    0xff00ffffffffffff,
    0x00ffffffffffffff,
};

const BitBoard MASK_RANK[8] = {
    0x00000000000000ff,
    0x000000000000ff00,
    0x0000000000ff0000,
    0x00000000ff000000,
    0x000000ff00000000,
    0x0000ff0000000000,
    0x00ff000000000000,
    0xff00000000000000,
};

const BitBoard CLEAR_FILE[8] = {
    0xfefefefefefefefe,
    0xfdfdfdfdfdfdfdfd,
    0xfbfbfbfbfbfbfbfb,
    0xf7f7f7f7f7f7f7f7,
    0xefefefefefefefef,
    0xdfdfdfdfdfdfdfdf,
    0xbfbfbfbfbfbfbfbf,
    0x7f7f7f7f7f7f7f7f,
};

const BitBoard CLEAR_FILE_AB = 18229723555195321596u;

const BitBoard CLEAR_FILE_GH = 4557430888798830399u;

const BitBoard MASK_FILE[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080,
};

const BitBoard MASK_SQUARE[64] = {
    0x0000000000000001,
    0x0000000000000002,
    0x0000000000000004,
    0x0000000000000008,
    0x0000000000000010,
    0x0000000000000020,
    0x0000000000000040,
    0x0000000000000080,
    0x0000000000000100,
    0x0000000000000200,
    0x0000000000000400,
    0x0000000000000800,
    0x0000000000001000,
    0x0000000000002000,
    0x0000000000004000,
    0x0000000000008000,
    0x0000000000010000,
    0x0000000000020000,
    0x0000000000040000,
    0x0000000000080000,
    0x0000000000100000,
    0x0000000000200000,
    0x0000000000400000,
    0x0000000000800000,
    0x0000000001000000,
    0x0000000002000000,
    0x0000000004000000,
    0x0000000008000000,
    0x0000000010000000,
    0x0000000020000000,
    0x0000000040000000,
    0x0000000080000000,
    0x0000000100000000,
    0x0000000200000000,
    0x0000000400000000,
    0x0000000800000000,
    0x0000001000000000,
    0x0000002000000000,
    0x0000004000000000,
    0x0000008000000000,
    0x0000010000000000,
    0x0000020000000000,
    0x0000040000000000,
    0x0000080000000000,
    0x0000100000000000,
    0x0000200000000000,
    0x0000400000000000,
    0x0000800000000000,
    0x0001000000000000,
    0x0002000000000000,
    0x0004000000000000,
    0x0008000000000000,
    0x0010000000000000,
    0x0020000000000000,
    0x0040000000000000,
    0x0080000000000000,
    0x0100000000000000,
    0x0200000000000000,
    0x0400000000000000,
    0x0800000000000000,
    0x1000000000000000,
    0x2000000000000000,
    0x4000000000000000,
    0x8000000000000000,
};

const BitBoard MASK_F1_TO_G1 = 0x60;

const BitBoard MASK_B1_TO_D1 = 0xe;

const BitBoard MASK_F8_TO_G8 = 0x6000000000000000;

const BitBoard MASK_B8_TO_D8 = 0xe00000000000000;

/**
 * TODO: write description
 */
static BitBoard generate_white_pawn_attack_mask(int square)
{
    BitBoard attacks = 0;

    attacks |= (MASK_SQUARE[square] << 7) & CLEAR_FILE[FILE_H];
    attacks |= (MASK_SQUARE[square] << 9) & CLEAR_FILE[FILE_A];

    return attacks;
}

/**
 * TODO: write description
 */
static BitBoard generate_black_pawn_attack_mask(int square)
{
    BitBoard attacks = 0;

    attacks |= (MASK_SQUARE[square] >> 9) & CLEAR_FILE[FILE_H];
    attacks |= (MASK_SQUARE[square] >> 7) & CLEAR_FILE[FILE_A];

    return attacks;
}

/**
 * TODO: write description
 */
static BitBoard generate_knight_attack_mask(int square)
{
    BitBoard attacks = 0;

    attacks |= (MASK_SQUARE[square] << 17) & CLEAR_FILE[FILE_A];
    attacks |= (MASK_SQUARE[square] << 10) & CLEAR_FILE_AB;
    attacks |= (MASK_SQUARE[square] >>  6) & CLEAR_FILE_AB;
    attacks |= (MASK_SQUARE[square] >> 15) & CLEAR_FILE[FILE_A];
    attacks |= (MASK_SQUARE[square] << 15) & CLEAR_FILE[FILE_H];
    attacks |= (MASK_SQUARE[square] <<  6) & CLEAR_FILE_GH;
    attacks |= (MASK_SQUARE[square] >> 10) & CLEAR_FILE_GH;
    attacks |= (MASK_SQUARE[square] >> 17) & CLEAR_FILE[FILE_H];

    return attacks;
}

/**
 * TODO: write description
 */
static BitBoard generate_king_attack_mask(int square)
{
    BitBoard attacks = 0;

    attacks |= (MASK_SQUARE[square] << 1) & CLEAR_FILE[FILE_A];
    attacks |= (MASK_SQUARE[square] << 1) & CLEAR_FILE[FILE_H];
    attacks |= ((MASK_SQUARE[square] | attacks) << 8) | ((MASK_SQUARE[square] | attacks) >> 8);

    return attacks;
}

/**
 * TODO: write description
 */
void lookup_tables_init()
{
    for (int square = 0; square < 64; square++)
    {
        MASK_PAWN_ATTACKS[WHITE][square] = generate_white_pawn_attack_mask(square);
        MASK_PAWN_ATTACKS[BLACK][square] = generate_black_pawn_attack_mask(square);

        MASK_KNIGHT_ATTACKS[square] = generate_knight_attack_mask(square);

        MASK_KING_ATTACKS[square] = generate_king_attack_mask(square);
    }
}
#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include "bitboard.h"

unsigned int popcount(Bitboard board)
{
	unsigned int count;

	for (count = 0; board; count++)
		board &= board - 1;

	return count;
}

void print_bitboard(Bitboard board)
{
	char buffer[21];

	for (int row = 0; row < 8; row++)
	{
		snprintf(buffer, 4, "%i |", 8 - row);

		for (int col = 0; col < 8; col++)
		{
			Bitboard position = (U64)1 << ((7 - row) * 8 + col);

			buffer[3 + col * 2] = ' ';
			if (board & position)
				buffer[(3 + col * 2) + 1] = '1';
			else
				buffer[(3 + col * 2) + 1] = '0';
		}

		buffer[19] = '\n';
		buffer[20] = '\0';
		printf(buffer);
	}

	strcpy(buffer, "   ----------------\n");
	printf(buffer);

	strcpy(buffer, "    A B C D E F G H\n");
	printf(buffer);
}

static const int bit_scan_forward_index[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan (bb != 0)
 * @return index (0..63) of least significant one bit
 */
static int bitScanForward(U64 bb) {
   const U64 debruijn64 = 0x03f79d71b4cb0a89;
   return bit_scan_forward_index[((bb ^ (bb-1)) * debruijn64) >> 58];
}

BitList bitboard_index(Bitboard board) {
	BitList bits;
	bits.size = 0;

	while (board) {
		bits.indices[bits.size++] = bitScanForward(board);

		board &= board - 1;
	}

	return bits;
}

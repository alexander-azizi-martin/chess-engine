#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include "bitboard.h"

static const int bit_scan_forward_index[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63,
};

/**
 * Fills the bitindicies with the indices of the 1s in the
 * bitboard.
 */
void bitboard_index(BitIndices *bits, Bitboard board) {
	bits->size = 0;

	while (board) {
		// Gets the index (0...63) of the LSB in board
		bits->indices[bits->size++] = bit_scan_forward_index[((board ^ (board-1)) * 0x03f79d71b4cb0a89) >> 58];

		board &= board - 1;
	}
}

/**
 * Counts the number of 1s in the given bitboard.
 */
int bitboard_count(Bitboard board)
{
	int count;

	for (count = 0; board; count++)
		board &= board - 1;

	return count;
}

/**
 * Prints a formated representation of the given bitboard.
 */
void bitboard_print(Bitboard board)
{
	char buffer[21];
	Bitboard index;

	for (int rank = 0; rank < 8; rank++)
	{
		snprintf(buffer, 4, "%i |", 8 - rank);

		for (int file = 0; file < 8; file++)
		{
			index = (U64)1 << ((7 - rank) * 8 + file);

			buffer[3 + file * 2] = ' ';
			buffer[(3 + file * 2) + 1] = (board & index) ? '1' : '0';
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
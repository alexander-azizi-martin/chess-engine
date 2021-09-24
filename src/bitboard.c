#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include "bitboard.h"
#include "lookup_tables.h"

/**
 * Returns the index of the LSB in the given board.
 */
inline int bitboard_scan_forward(BitBoard board) 
{
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

	return bit_scan_forward_index[((board ^ (board-1)) * 0x03f79d71b4cb0a89) >> 58];
}

/**
 * Returns the index of the LSB and sets it to 0 or -1 no bits are set.
 */
int bitboard_pop(BitBoard *board)
{
	if (*board)
	{
		int index = bitboard_scan_forward(*board);

		*board &= *board - 1;

		return index;
	}

	return -1;
}

/**
 * Used to iterate over the indexies of a bitboard. For the initial call, pass in the bitboard to iterate over, then 
 * pass NULL for all subsequent calls. Returns -1 when finished iterating.
 * 
 * Note: This function does not work recursively and only keeps track of a single bitboard at once
 * 
 * ex:
 * for(int i = bitboard_iter(board); i != -1; i = bitboard_iter(NULL)) {}
 */
int bitboard_iter(BitBoard *board)
{
	static BitBoard saved_board;
	saved_board = (board == NULL) ? saved_board : *board;

	if (saved_board)
	{
		int index = bitboard_scan_forward(saved_board);

		saved_board &= saved_board - 1;

		return index;
	}

	return -1;
}

/**
 * Returns the number of non zero bits in the given bitboard.
 */
int bitboard_count(BitBoard board)
{
	int count;

	for (count = 0; board; count++)
		board &= board - 1;

	return count;
}

/**
 * Prints a formated representation of the given bitboard.
 */
void bitboard_print(BitBoard board)
{
	for (int rank = RANK_8; rank >= RANK_1; rank--)
	{
		printf("%i |", rank + 1);
		for (int file = FILE_A; file <= FILE_H; file++)
		{
			printf(" %c", (board & FileRankToSquare(file, rank)) ? '1' : '0');
		}
		printf("\n");
	}

	printf("   ----------------\n");
	printf("    A B C D E F G H\n");
}
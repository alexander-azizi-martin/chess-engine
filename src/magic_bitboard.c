/*
I found the following explanation of magic bitboards helpful (https://stackoverflow.com/a/30703816):

First, some terminology:
- blocker mask: A bitboard containing all squares that can block a piece, for a given piece type and the square 
  the piece is on. It excludes terminating edge squares because they always block.
- blocker board: A bitboard containing occupied squares. It only has squares which are also in the blocker mask.
- move board: A bitboard containing all squares a piece can move to, given a piece type, a square, and a blocker 
  board. It includes terminating edge squares if the piece can move there.

Example for a rook on the e4 square, and there are some random pieces on e2, e5, e7, b4, and c4.

 The blocker mask        A blocker board         The move board
 0 0 0 0 0 0 0 0         0 0 0 0 0 0 0 0         0 0 0 0 0 0 0 0 
 0 0 0 0 1 0 0 0         0 0 0 0 1 0 0 0         0 0 0 0 0 0 0 0 
 0 0 0 0 1 0 0 0         0 0 0 0 0 0 0 0         0 0 0 0 0 0 0 0 
 0 0 0 0 1 0 0 0         0 0 0 0 1 0 0 0         0 0 0 0 1 0 0 0 
 0 1 1 1 0 1 1 0         0 1 1 0 0 0 0 0         0 0 1 1 0 1 1 1 
 0 0 0 0 1 0 0 0         0 0 0 0 0 0 0 0         0 0 0 0 1 0 0 0 
 0 0 0 0 1 0 0 0         0 0 0 0 1 0 0 0         0 0 0 0 1 0 0 0 
 0 0 0 0 0 0 0 0         0 0 0 0 0 0 0 0         0 0 0 0 0 0 0 0 

Some things to note:
- The blocker mask is always the same for a given square and piece type (either rook or bishop).
- Blocker boards include friendly & enemy pieces, and it is a subset of the blocker mask.
- The resulting move board may include moves that capture your own pieces, however these moves are easily 
  removed afterward: moveboard &= ~friendly_pieces)

The goal of the magic numbers method is to very quickly look up a pre-calculated move board for a given blocker 
board. Otherwise you'd have to (slowly) calculate the move board every time. This only applies to sliding pieces, 
namely the rook and bishop. The queen is just a combination of the rook and bishop.

Magic numbers can be found for each square & piece type combo. To do this, you have to calculate every possible 
blocker board variation for each square/piece combo.
*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "magic_bitboard.h"
#include "lookup_tables.h"
#include "bitboard.h"

/**
 * TODO: write description
 */
static BitBoard generate_blocker_board(int index, BitBoard blocker_mask)
{
    BitBoard blocker_board = 0ULL;
    
    for (int i = 0, len = bitboard_count(blocker_mask); i < len; i++)
    {
        int square = bitboard_pop(&blocker_mask);
        
        if (index & (1 << i))
            blocker_board |= (U64) 1 << square;
    }
    
    return blocker_board;
}

/**
 * Returns a mask containing all the squares that can block a bishop on 
 * the given square.
 */
static BitBoard generate_bishop_blocker_mask(int square)
{
	int square_rank = square / 8;
	int square_file = square % 8;

	BitBoard blockers = 0;

	// Calculating bishop moves north east
	int rank, file;
	for (rank = square_rank + 1, file = square_file + 1; rank < 7 && file < 7; rank++, file++)
		blockers |= FileRankToSquare(file, rank);
	// Calculating bishop moves south west
	for (rank = square_rank - 1, file = square_file - 1; rank > 0 && file > 0; rank--, file--)
		blockers |= FileRankToSquare(file, rank);
	// Calculating bishop moves north west
	for (rank = square_rank + 1, file = square_file - 1; rank < 7 && file > 0; rank++, file--)
		blockers |= FileRankToSquare(file, rank);
	// Calculating bishop moves south east
	for (rank = square_rank - 1, file = square_file + 1; rank > 0 && file < 7; rank--, file++)
		blockers |= FileRankToSquare(file, rank);

	return blockers;
}

/**
 * Returns a mask containing all the squares that can block a rook on the 
 * given square.
 */
static BitBoard generate_rook_blocker_mask(int square)
{
	int square_rank = square / 8;
	int square_file = square % 8;

	BitBoard blockers = 0;

	// Calculating rook moves north
	int rank, file;
	for (rank = square_rank + 1; rank < 7; rank++)
		blockers |= FileRankToSquare(square_file, rank);
	// Calculating rook moves south
	for (rank = square_rank - 1; rank > 0; rank--)
		blockers |= FileRankToSquare(square_file, rank);
	// Calculating rook moves west
	for (file = square_file - 1; file > 0; file--)
		blockers |= FileRankToSquare(file, square_rank);
	// Calculating rook moves east
	for (file = square_file + 1; file < 7; file++)
		blockers |= FileRankToSquare(file, square_rank);

	return blockers;
}

/**
 * Returns a mask containing all the squares that a bishop can attack based
 * off what square it is on and the pieces blocking it.
 */
static BitBoard generate_bishop_attack_mask(int square, BitBoard block)
{
	int square_rank = square / 8;
	int square_file = square % 8;

	BitBoard attacks = 0;

	// Calculating bishop moves north east
	int rank, file;
	for (rank = square_rank + 1, file = square_file + 1; rank < 8 && file < 8; rank++, file++)
	{
		attacks |= FileRankToSquare(file, rank);
		if (FileRankToSquare(file, rank) & block) break;
	}
	// Calculating bishop moves south west
	for (rank = square_rank - 1, file = square_file - 1; rank >= 0 && file >= 0; rank--, file--)
	{
		attacks |= FileRankToSquare(file, rank);
		if (FileRankToSquare(file, rank) & block) break;
	}
	// Calculating bishop moves north west
	for (rank = square_rank + 1, file = square_file - 1; rank < 8 && file >= 0; rank++, file--)
	{
		attacks |= FileRankToSquare(file, rank);
		if (FileRankToSquare(file, rank) & block) break;
	}
	// Calculating bishop moves south east
	for (rank = square_rank - 1, file = square_file + 1; rank >= 0 && file < 8; rank--, file++)
	{
		attacks |= FileRankToSquare(file, rank);
		if (FileRankToSquare(file, rank) & block) break;
	}

	return attacks;
}

/**
 * Returns a mask containing all the squares that a rook can attack based off 
 * what square it is on and the pieces blocking it.
 */
static BitBoard generate_rook_attack_mask(int square, BitBoard block)
{
	int square_rank = square / 8;
	int square_file = square % 8;

	BitBoard attacks = 0;

	// Calculating rook moves north
	int rank, file;
	for (rank = square_rank + 1; rank < 8; rank++)
	{
		attacks |= FileRankToSquare(square_file, rank);
		if (FileRankToSquare(square_file, rank) & block) break;
	}
	// Calculating rook moves south
	for (rank = square_rank - 1; rank >= 0; rank--)
	{
		attacks |= FileRankToSquare(square_file, rank);
		if (FileRankToSquare(square_file, rank) & block) break;
	}
	// Calculating rook moves west
	for (file = square_file - 1; file >= 0; file--)
	{
		attacks |= FileRankToSquare(file, square_rank);
		if (FileRankToSquare(file, square_rank) & block) break;
	}
	// Calculating rook moves east
	for (file = square_file + 1; file < 8; file++)
	{
		attacks |= FileRankToSquare(file, square_rank);
		if (FileRankToSquare(file, square_rank) & block) break;
	}

	return attacks;
}

/**
 * Returns a pseudo random 64 bit number as a possible magic number candaidate.
 */
static BitBoard magic_number_candidate()
{
    BitBoard magic_number = 0;

	// Anding the rand() to decrease the number of non zero bits
    magic_number |= ((U64) (rand() & rand()) & 0xffff);
    magic_number |= ((U64) (rand() & rand()) & 0xffff) << 16;
    magic_number |= ((U64) (rand() & rand()) & 0xffff) << 32;
    magic_number |= ((U64) (rand() & rand()) & 0xffff) << 48;

    return magic_number;
}

/**
 * Generates a working magic number on the given square by trail and error. If
 * bishop_magic_number is set to true it will generate a magic number for bishops,
 * else a magic number for rooks.
 */
U64 generate_magic_number(int square, bool bishop_magic_number)
{
	BitBoard blocker_boards[4096];
	BitBoard attacks[4096];
	BitBoard used_attacks[4096];

	BitBoard blocker_mask = bishop_magic_number 
		? generate_bishop_blocker_mask(square) 
		: generate_rook_blocker_mask(square);

	int num_bits = bitboard_count(blocker_mask);
	int blocker_combinations = 1 << num_bits;
	
    for (int index = 0; index < blocker_combinations; index++)
    {
        blocker_boards[index] = generate_blocker_board(index, blocker_mask);
        
        attacks[index] = bishop_magic_number 
			? generate_bishop_attack_mask(square, blocker_boards[index]) 
			: generate_rook_attack_mask(square, blocker_boards[index]);
    }
    
    // Guesses a maximum of 1,000,000 magic numbers
    for (int i = 0; i < 1000000; i++)
    {
        U64 magic_number = magic_number_candidate();
        
		memset(used_attacks, 0, sizeof(used_attacks));

		// Validates maggic number candidate by checking if it maps each blocker board to a unique index
		bool valid_magic_number = true;
		for (int i = 0; i < blocker_combinations && valid_magic_number; i++)
		{
			int magic_index = (blocker_boards[i] * magic_number) >> (64 - num_bits);

			if (used_attacks[magic_index] == 0)
				used_attacks[magic_index] = attacks[i];
			else
				valid_magic_number = false;
		}

		if (valid_magic_number)
			return magic_number;
    }
	
	printf("Couldn't generate a magic number in time.\n");
    exit(1);
}

/**
 * Initializes all the information needed to 
 */
void magic_bitboards_init()
{
	int bishop_offset = 0, rook_offset = 0;
	for (int square = 0; square < 64; square++)
	{
		BitBoard blocker_mask = generate_bishop_blocker_mask(square);
		MAGIC_BISHOP_TABLE[square].attack_table = &MASK_BISHOP_ATTACKS[bishop_offset];
		MAGIC_BISHOP_TABLE[square].blocker_mask = blocker_mask;
		MAGIC_BISHOP_TABLE[square].magic_number = generate_magic_number(square, true);
		MAGIC_BISHOP_TABLE[square].shift = 64 - bitboard_count(blocker_mask);

		for (int i = 0, len = 1 << bitboard_count(blocker_mask); i < len; i++)
		{
			BitBoard blocker_board = generate_blocker_board(i, blocker_mask);

			int magic_index = (blocker_board * MAGIC_BISHOP_TABLE[square].magic_number) >> MAGIC_BISHOP_TABLE[square].shift;

			MASK_BISHOP_ATTACKS[bishop_offset + magic_index] = generate_bishop_attack_mask(square, blocker_board);
		}

		bishop_offset += 1 << bitboard_count(blocker_mask);

		blocker_mask = generate_rook_blocker_mask(square);
		MAGIC_ROOK_TABLE[square].attack_table = &MASK_ROOK_ATTACKS[rook_offset];
		MAGIC_ROOK_TABLE[square].blocker_mask = blocker_mask;
		MAGIC_ROOK_TABLE[square].magic_number = generate_magic_number(square, false);
		MAGIC_ROOK_TABLE[square].shift = 64 - bitboard_count(blocker_mask);

		for (int i = 0, len = 1 << bitboard_count(blocker_mask); i < len; i++)
		{
			BitBoard blocker_board = generate_blocker_board(i, blocker_mask);

			int magic_index = (blocker_board * MAGIC_ROOK_TABLE[square].magic_number) >> MAGIC_ROOK_TABLE[square].shift;

			MASK_ROOK_ATTACKS[rook_offset + magic_index] = generate_rook_attack_mask(square, blocker_board);
		}

		rook_offset += 1 << bitboard_count(blocker_mask);
	}
}

BitBoard lookup_bishop_attacks(int square, BitBoard occupied_squares) 
{
	BitBoard blocker_board = occupied_squares & MAGIC_BISHOP_TABLE[square].blocker_mask;

	blocker_board *= MAGIC_BISHOP_TABLE[square].magic_number;
	blocker_board >>= MAGIC_BISHOP_TABLE[square].shift;

	return MAGIC_BISHOP_TABLE[square].attack_table[blocker_board];
}

BitBoard lookup_rook_attacks(int square, BitBoard occupied_squares) 
{
	BitBoard blocker_board = occupied_squares & MAGIC_ROOK_TABLE[square].blocker_mask;

	blocker_board *= MAGIC_ROOK_TABLE[square].magic_number;
	blocker_board >>= MAGIC_ROOK_TABLE[square].shift;

	return MAGIC_ROOK_TABLE[square].attack_table[blocker_board];
}
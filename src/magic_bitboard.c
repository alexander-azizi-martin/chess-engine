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

static U64 BISHOP_MAGIC_NUMBERS[64] = {
	342278278997886096,
	4735818700098379792,
	1011102101409898776,
	397465765584437540,
	2399297795453616129,
	4730470727087170649,
	4618724061112108040,
	4877963038750278166,
	2937200594697462273,
	1776995724455837953,
	26396873327682,
	468405427308070080,
	90725111337845764,
	4630828550539706417,
	2596443984008974848,
	577131742236254409,
	3981468082335647136,
	964100405707539971,
	5316499978726686800,
	183522002666127360,
	5010817964918902824,
	1157989231571765312,
	2756783793406751755,
	652177557027885081,
	1235148156379989007,
	439136981676073344,
	146375788303089857,
	1982710010895073282,
	689614311517081601,
	2317117401449170560,
	1316494131701745664,
	505534557450162985,
	937881494349173027,
	2308117015827322881,
	3045560107355342944,
	4647741238105475584,
	2324985790123475208,
	1196275291040270352,
	617047158499197952,
	5076190653368107520,
	109218991314583576,
	4661518917916434484,
	1585840206484344840,
	126137010166450184,
	5440955289545237505,
	5513057000820310528,
	4905000553542656944,
	4794125887979274688,
	579857162279322624,
	2778167099863861259,
	112950922623320097,
	871464240904929813,
	2380280050617680962,
	4687794264976003074,
	594554638227866644,
	614761363751576256,
	4649122362146164741,
	6485198860955697704,
	3607684623548878880,
	83404597667040261,
	4107287275456234496,
	1244412006003377284,
	653059708129183809,
	9042392518763426,
};

static U64 ROOK_MAGIC_NUMBERS[64] = {
	324263983811018752,
	702564702977262656,
	4647732407775010820,
	648532158956765280,
	7638113850650275872,
	108090789262797312,
	4755810707004457256,
	2377901978496795522,
	1460010846024368324,
	612559918604484684,
	554224299473389315,
	8797218991532802624,
	145522803477844224,
	291045177597231216,
	2342997998753433616,
	1189231815257243778,
	9324408501977096,
	2886812034163277888,
	6341633425446801440,
	5765823583062343712,
	451486412478873728,
	1156018829197846662,
	4755814400778043937,
	1441734622097523972,
	577096295796244609,
	2451192411980824832,
	1186135697888725376,
	1189232485272670209,
	18859042483470356,
	30399349042208776,
	2323968509937717254,
	289427203148743489,
	3513441028726063752,
	45106412279309441,
	4651657123548107200,
	363103442618556432,
	4685996035066038572,
	4639833585019330696,
	2310707321870750256,
	650278115943121921,
	1188953462727000072,
	5661044660265566208,
	2747759758835580960,
	379146862349451298,
	3531425739876401169,
	4630263401535373380,
	162145001494806552,
	2490495593317793803,
	1261009155372680960,
	1153273361221104384,
	5069083079457980672,
	306635101707436288,
	2900600004504719616,
	226868900896440832,
	550888346353665024,
	6090107533870239232,
	136586837606662178,
	1227267182757412993,
	1297600192797876290,
	524990796238188545,
	5424023111267779602,
	180707553660111638,
	1441242075081412612,
	479705395523756738,
};

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
		// MAGIC_BISHOP_TABLE[square].magic_number = generate_magic_number(square, true);
		MAGIC_BISHOP_TABLE[square].magic_number = BISHOP_MAGIC_NUMBERS[square];
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
		// MAGIC_ROOK_TABLE[square].magic_number = generate_magic_number(square, false);
		MAGIC_ROOK_TABLE[square].magic_number = ROOK_MAGIC_NUMBERS[square];
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
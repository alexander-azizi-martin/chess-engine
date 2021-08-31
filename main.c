#include <stdio.h>
#include "defs.h"
#include "chessboard.h"
#include "move.h"
#include "bitboard.h"
#include "lookup_tables.h"

int main(void)
{
	// ChessBoard board;

	// chessboard_init(&board, "rnbqkbnr/ppp1pppp/8/3p4/4P1K1/3N4/PPNP1PPP/R1B2B1R w kq - 0 1");
	// chessboard_print(&board);

	// GeneratedMoves moves = {.moves = 0};
	// chessboard_generate_king_moves(&board, &moves);

	// BitBoard king_moves = 0;

	// for (int i = 0; i < moves.size; i++)
	// {
	// 	king_moves |= MASK_SQUARE[moves.moves[i].target];
	// }

	// bitboard_print(king_moves);

	bitboard_print(0);

	return 0;
}
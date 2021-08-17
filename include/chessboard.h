#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"

typedef struct
{
	Bitboard pieces[2][6];

	/* Commonly derived positions */
	Bitboard all_white_pieces;
	Bitboard all_black_pieces;
	Bitboard all_pieces;

	/* Moves */
	Move moves_played[2048];
	int num_moves;
} ChessBoard;

ChessBoard *chessboard_init(string fen_str);

void chessboard_print(ChessBoard *board);

void chessboard_make_move(ChessBoard *board, Move move);

void chessboard_undo_move(ChessBoard *board);

#endif
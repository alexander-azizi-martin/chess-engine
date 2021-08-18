#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"

typedef struct
{
	Bitboard pieces[14];

	Bitboard occupied_squares;
	Bitboard empty_squares;

	Move moves_played[2048];
	int num_moves;
} ChessBoard;

void chessboard_init(ChessBoard *board, string fen_str);

Piece chessboard_get_piece(ChessBoard *board, Bitboard index);

void chessboard_make_move(ChessBoard *board, Move move);

void chessboard_undo_move(ChessBoard *board);

void chessboard_print(ChessBoard *board);

Piece fen_to_piece(char f);

char piece_to_fen(Piece p);

#endif
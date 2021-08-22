#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"

typedef enum 
{
	WHITE_KING_SIDE = 1,
	WHITE_QUEEN_SIDE = 2,
	BLACk_KING_SIDE = 4,
	BLACK_QUEEN_SIDE = 8,
} CastlePermissions;

typedef struct
{
	Bitboard pieces[14];

	Bitboard occupied_squares;
	Bitboard empty_squares;
	Bitboard en_passent_target;

	int castle_permission;

	int num_full_moves;
	int num_half_moves;

	int current_color;
} ChessBoard;

void chessboard_init(ChessBoard *board, char *fen_str);

Piece chessboard_get_piece(ChessBoard *board, Bitboard index);

void chessboard_make_move(ChessBoard *board, Move move);

void chessboard_undo_move(ChessBoard *board);

void chessboard_generate_pawn_moves(ChessBoard *board, GeneratedMoves *moves);

void chessboard_print(ChessBoard *board);

Piece fen_to_piece(char f);

char piece_to_fen(Piece p);

#endif
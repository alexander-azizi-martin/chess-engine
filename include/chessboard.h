#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "defs.h"
#include "bitboard.h"
#include "move.h"

typedef enum 
{
	WHITE_KING_SIDE = 1,
	WHITE_QUEEN_SIDE = 2,
	BLACK_KING_SIDE = 4,
	BLACK_QUEEN_SIDE = 8,
} CastlePermissions;

typedef struct
{
	Move move;
	U8 en_passent_target;
	U8 castle_permission;
} MoveInfo;

typedef struct
{
	BitBoard pieces[14];

	BitBoard occupied_squares;
	BitBoard empty_squares;
	BitBoard available_squares;
	BitBoard en_passent;

	int castle_permission;

	int num_full_moves;
	int num_half_moves;

	int current_color;

	MoveInfo move_history[2048];
	int num_moves;
} ChessBoard;

void chessboard_init(ChessBoard *board, char *fen_str);

Piece chessboard_get_piece(ChessBoard *board, BitBoard square_mask);

int chessboard_make_move(ChessBoard *board, Move move);

void chessboard_undo_move(ChessBoard *board);

void chessboard_generate_moves(ChessBoard *board, MoveList *list);

void chessboard_print(ChessBoard *board);

Piece fen_to_piece(char f);

char piece_to_fen(Piece p);

#endif
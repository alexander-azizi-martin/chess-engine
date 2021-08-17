#ifndef MOVE_H
#define MOVE_H

#include <stdbool.h>
#include "defs.h"
#include "chessboard.h"

typedef U32 Move;

typedef enum
{
	QUIET_MOVE,
	PAWN_PUSH,
	KING_CASTLE,
	QUEEN_CASTLE,
	EN_PASSANT,
	KNIGHT_PROMOTION,
	BISHOP_PROMOTION,
	ROCK_PROMOTION,
} MoveFlags;

Move move_init(Square origin, Square target, PieceType piece, PieceType capturedPiece, MoveFlags flags);

Square move_get_origin(Move move);

Square move_get_target(Move move);

PieceType move_get_piece(Move move);

PieceType move_get_capture(Move move);

bool move_is_capture(Move move);

#endif
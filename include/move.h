#ifndef MOVE_H
#define MOVE_H

#include <stdbool.h>
#include "defs.h"

typedef enum
{
	QUIET_MOVE,
	PAWN_PUSH,
	KING_CASTLE,
	QUEEN_CASTLE,
	ROOK_PROMOTION,
	KNIGHT_PROMOTION,
	BISHOP_PROMOTION,
	QUEEN_PROMOTION,
} MoveFlags;

typedef struct 
{
	U8 origin;
	U8 target;
	U8 piece;
	U8 captured_piece;
	U8 flags;
} Move;

typedef struct 
{
	Move moves[256];
	U8 size;
} GeneratedMoves;

#endif
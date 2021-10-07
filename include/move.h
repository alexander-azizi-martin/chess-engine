#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

typedef enum
{
	NORMAL_MOVE,
	KING_CASTLE,
	QUEEN_CASTLE,
	ROOK_PROMOTION,
	KNIGHT_PROMOTION,
	BISHOP_PROMOTION,
	QUEEN_PROMOTION,
} MoveType;

typedef struct 
{
	U8 origin;
	U8 target;
	U8 piece : 4;
	U8 captured_piece : 4;
	U8 move_type;
} Move;

typedef struct
{
	Move moves[256];
	U8 size;
} MoveList;

typedef struct
{
	Move moves[2048];
	int size;
} MoveHistory;

#endif
#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

typedef enum
{
	KING_CASTLE = 1,
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
	U8 piece : 4;
	U8 captured_piece : 4;
	U8 flag;
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
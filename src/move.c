#include "move.h"

Move move_init(Square origin, Square target, PieceType piece, PieceType capturedPiece, MoveFlags flags)
{
	return ((flags & 0xf) << 20) | ((piece & 0xf) << 16) | ((piece & 0xf) << 12) | ((target & 0x3f) << 6) | (origin & 0x3f);
}

Square move_get_origin(Move move)
{
	return move & 0x3f;
}

Square move_get_target(Move move)
{
	return (move >> 6) & 0x3f;
}

PieceType move_get_piece(Move move)
{
	return (move >> 12) & 0xf;
}

PieceType move_get_capture(Move move)
{
	return (move >> 16) & 0xf;
}

MoveFlags move_get_flags(Move move)
{
	return (move >> 20) & 0xf;
}

bool move_is_capture(Move move)
{
	return (move_get_capture(move) & QUIET_MOVE) != 0;
}

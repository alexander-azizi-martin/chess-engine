#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <stdbool.h>
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
    U64 position_key;
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

    U64 position_key;

    MoveInfo move_history[2048];
    int num_moves;
} ChessBoard;

/**
 * Initializes a chessboard's pieces with a fen stirng.
 **/
void chessboard_init(ChessBoard *board, char *fen_str);

/**
 * Returns a unique key based on the board's position.
 **/
U64 chessboard_hash(ChessBoard *board);

/**
 * Initializes the keys used to hash a chessboard.
 **/
void chessboard_init_keys(void);

/**
 * Returns the piece on the given square.
 **/
Piece chessboard_get_piece(ChessBoard *board, BitBoard square_mask);

/**
 * Returns whether the given square is being attacked by an opponent
 * piece.
 **/
bool chessboard_squared_attacked(ChessBoard *board, int target);

/**
 * Updates the chessboard's pieces after a piece is moved.
 **/
void chessboard_move_piece(ChessBoard *board, Move move);

/**
 * Updates the chessboard's pieces after the given pseudo legal move is played. 
 * Returns whether the move is legal. If the move is not legal the board is not
 * updated.
 **/
bool chessboard_make_move(ChessBoard *board, Move move);

/**
 * Updates the chessboard's pieces after undoing the last moved played.
 **/
void chessboard_undo_move(ChessBoard *board);

/**
 * Generates all possible pseudo legal moves in a given possition 
 * and adds them to the given MoveList.
 **/
void chessboard_generate_moves(ChessBoard *board, MoveList *list);

/**
 * Prints a formated representation of a chessboard.
 **/
void chessboard_print(ChessBoard *board);

Piece fen_to_piece(char f);

char* piece_to_fen(Piece p);

#endif
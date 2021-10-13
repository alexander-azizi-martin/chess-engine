#ifndef SEARCH_H
#define SEARCH_H

#include "chessboard.h"
#include "move.h"

int search_evaluation(ChessBoard *board);

int search_negamax(ChessBoard *board, int depth, int alpha, int beta);

Move search_position(ChessBoard *board);

#endif
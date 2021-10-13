#include <limits.h>
#include <stdio.h>
#include <math.h>
#include "search.h"

int search_evaluation(ChessBoard *board)
{
    static const int PIECE_VALUE[] = {0, 0, 10, 50, 30, 30, 90, 0, 10, 50, 30, 30, 90, 0};

    // Loops through the white colors
    int white_score = 0;
    for (Piece piece = WHITE_PAWNS; piece <= WHITE_KING; piece++)
    {
        white_score += bitboard_count(board->pieces[piece]) * PIECE_VALUE[piece];
    }

    // Loops through the black colors
    int black_score = 0;
    for (int piece = BLACK_PAWNS; piece <= BLACK_KING; piece++)
    {
        black_score += bitboard_count(board->pieces[piece]) * PIECE_VALUE[piece];
    }

    return board->current_color == WHITE 
        ? white_score - black_score 
        : black_score - white_score;
}

int search_negamax(ChessBoard *board, int depth, int alpha, int beta)
{
    if (depth == 0)
        return search_evaluation(board);
    
    int num_played_moves = 0;

    int score = INT_MIN;

    MoveList list;
    chessboard_generate_moves(board, &list);
    for (int i = 0; i < list.size; i++)
    {
        if (chessboard_make_move(board, list.moves[i]))
        {
            score = fmax(score, -search_negamax(board, depth - 1, -beta, -alpha));
            alpha = fmax(score, alpha);

            chessboard_undo_move(board);

            if(alpha >= beta)
                break;

            num_played_moves++;
        }
    }

    // The current player is in check or stale mate
    if (num_played_moves == 0)
    {
        int king_square = bitboard_scan_forward(board->current_color == WHITE ? WHITE_KING : BLACK_KING);

        // Checkmate
        if (chessboard_squared_attacked(board, bitboard_scan_forward(king_square)))
            return INT_MIN;
        // Stalemate
        else
            return 0;
    }

    return score;
}

Move search_position(ChessBoard *board)
{
    Move best_move;
    int best_score = INT_MIN;

    MoveList list;
    chessboard_generate_moves(board, &list);
    for (int i = 0; i < list.size; i++)
    {
        if (chessboard_make_move(board, list.moves[i]))
        {
            int new_score = -search_negamax(board, 4, INT_MIN, INT_MAX);

            if (best_score < new_score)
            {
                best_move = list.moves[i];
                best_score = new_score;
            }

            chessboard_undo_move(board);
        }
    }

    printf("Best score %i\n", best_score);

    return best_move;
}
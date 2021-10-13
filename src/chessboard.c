#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "chessboard.h"
#include "lookup_tables.h"
#include "magic_bitboard.h"

/**
 * Initializes a chessboard's position with a fen stirng.
 */
void chessboard_init(ChessBoard *board, char *fen_str)
{
    // Clears board
    memset(board, 0, sizeof(ChessBoard));

    // Copies fen_str into a local variables to avoid modifing it through strtok
    char fen_cpy[100];
    strncpy(fen_cpy, fen_str, sizeof(fen_cpy));

    // Parses pieces in fen string
    char *token = strtok(fen_cpy, " ");
    for (int i = 0, rank = RANK_8, file = FILE_A; token[i] != '\0'; i++)
    {
        if (token[i] == '/')
        {
            rank--;
            file = FILE_A;
        }
        else if (isdigit(token[i]))
        {
            file += token[i] - '0';
        }
        else
        {
            // Places the piece on the board
            Piece piece_type = fen_to_piece(token[i]);
            Piece piece_color = PieceColor(piece_type);

            board->pieces[piece_type] |= FileRankToSquare(file, rank);
            board->pieces[piece_color] |= FileRankToSquare(file, rank);
            file++;
        }
    }

    // Parses current player in fen string
    token = strtok(NULL, " ");
    board->current_color = (token[0] == 'w') ? WHITE : BLACK;
    
    // Parses castling permission in fen string
    token = strtok(NULL, " ");
    for (int i = 0; token[0] != '-' && token[i] != '\0'; i++) 
    {
        switch (token[i])
        {
            case 'K':
                board->castle_permission |= WHITE_KING_SIDE;
                break;
            case 'Q':
                board->castle_permission |= WHITE_QUEEN_SIDE;
                break;
            case 'k':
                board->castle_permission |= BLACK_KING_SIDE;
                break;
            case 'q':
                board->castle_permission |= BLACK_QUEEN_SIDE;
                break;
        }
    }

    // Parses en passent square
    token = strtok(NULL, " ");
    if (token[0] != '-') 
        board->en_passent |= FileRankToSquare(token[0] - 'a', token[1] - '1');

    // Parses full and half move clock
    token = strtok(NULL, " ");
    board->num_half_moves = atoi(token);
    token = strtok(NULL, " ");
    board->num_full_moves = atoi(token);

    board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
    board->empty_squares = ~board->occupied_squares;
    board->available_squares = ~board->pieces[board->current_color];
}

/**
 * Returns the piece on the given square.
 */
Piece chessboard_get_piece(ChessBoard *board, BitBoard square_mask) 
{
    if (board->empty_squares & square_mask)
        return EMPTY;
    
    Piece start = (board->pieces[WHITE] & square_mask) ? WHITE_PAWNS : BLACK_PAWNS;
    for (int piece = start; piece < start + 6; piece++) 
    {
        if (board->pieces[piece] & square_mask)
            return piece;
    }

    return EMPTY;
}

/**
 * Returns a bitboard containing the squares attacked by any opponent pieces.
 */
bool chessboard_squared_attacked(ChessBoard *board, int target)
{
    // Shift to change piece color from white to black
    int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

    if (MASK_PAWN_ATTACKS[board->current_color][target] & board->pieces[BLACK_PAWNS - color_shift])
        return true;
    if (MASK_KNIGHT_ATTACKS[target] & board->pieces[BLACK_KNIGHTS - color_shift])
        return true;
    if (MASK_KING_ATTACKS[target] & board->pieces[BLACK_KING - color_shift])
        return true;
    if (lookup_bishop_attacks(target, board->occupied_squares) & (board->pieces[BLACK_QUEENS - color_shift] | board->pieces[BLACK_BISHOPS - color_shift]))
        return true;
    if (lookup_rook_attacks(target, board->occupied_squares) & (board->pieces[BLACK_QUEENS - color_shift] | board->pieces[BLACK_ROOKS - color_shift]))
        return true;

    return false;
}

/**
 * TODO: write description
 */
int chessboard_make_move(ChessBoard *board, Move move)
{
    board->move_history[board->num_moves].move = move;
    board->move_history[board->num_moves].castle_permission = board->castle_permission;
    board->move_history[board->num_moves].en_passent_target = (board->en_passent) 
        ? bitboard_scan_forward(board->en_passent) 
        : -1;
    board->num_moves++;

    // Shift to change piece color from white to black
    int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

    // Updates the new position of the piece
    switch (move.move_type)
    {
        case ROOK_PROMOTION:
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case KNIGHT_PROMOTION:
            board->pieces[WHITE_KNIGHTS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case BISHOP_PROMOTION:
            board->pieces[WHITE_BISHOPS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case QUEEN_PROMOTION:
            board->pieces[WHITE_QUEENS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case KING_CASTLE:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target + 1] | MASK_SQUARE[move.target - 1];
            board->pieces[board->current_color] ^= MASK_SQUARE[move.target + 1] | MASK_SQUARE[move.target - 1];
            break;
        case QUEEN_CASTLE:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
            board->pieces[board->current_color] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
            break;
        default:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            break;
    }

    board->pieces[move.piece] ^= MASK_SQUARE[move.origin];
    board->pieces[board->current_color] ^= MASK_SQUARE[move.target];
    
    if (move.captured_piece != EMPTY)
    {
        board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target];
        board->pieces[!board->current_color] ^= MASK_SQUARE[move.target];
    }

    board->en_passent = 0;
    board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
    board->empty_squares = ~board->occupied_squares;
    board->available_squares = ~board->pieces[!board->current_color];

    // Invalid move if king is being attacked
    if (chessboard_squared_attacked(board, bitboard_scan_forward(board->pieces[WHITE_KING + color_shift])))
    {
        chessboard_undo_move(board);
        return -1;
    }

    board->current_color = !board->current_color;

    // Updates castling permissions and en passent square
    switch (move.piece)
    {
        case WHITE_ROOKS: 
            if (move.origin == 0 && board->castle_permission & WHITE_QUEEN_SIDE)
                board->castle_permission ^= WHITE_QUEEN_SIDE;
            else if (move.origin == 7 && board->castle_permission & WHITE_KING_SIDE)
                board->castle_permission ^= WHITE_KING_SIDE;
            break;
        case BLACK_ROOKS:
            if (move.origin == 56 && board->castle_permission & BLACK_QUEEN_SIDE)
                board->castle_permission ^= BLACK_QUEEN_SIDE;
            else if (move.origin == 63 && board->castle_permission & BLACK_KING_SIDE)
                board->castle_permission ^= BLACK_KING_SIDE;
            break;
        case WHITE_KING:
            if (board->castle_permission & WHITE_KING_SIDE)
                board->castle_permission ^= WHITE_KING_SIDE;
            if (board->castle_permission & WHITE_QUEEN_SIDE)
                board->castle_permission ^= WHITE_QUEEN_SIDE;
            break;
        case BLACK_KING:
            if (board->castle_permission & BLACK_KING_SIDE)
                board->castle_permission ^= BLACK_KING_SIDE;
            if (board->castle_permission & BLACK_QUEEN_SIDE)
                board->castle_permission ^= BLACK_QUEEN_SIDE;
            break;
        case WHITE_PAWNS:
            if (move.target - move.origin == 16)
                board->en_passent = MASK_SQUARE[move.target - 8];
            break;
        case BLACK_PAWNS:
            if (move.target - move.origin == -16)
                board->en_passent = MASK_SQUARE[move.target + 8];
            break;
    }

    return 1;
}

void chessboard_undo_move(ChessBoard *board)
{
    board->num_moves--;
    Move move = board->move_history[board->num_moves].move;
    board->current_color = PieceColor(move.piece);

    // Shift to change piece color from white to black
    int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

    switch (move.move_type)
    {
        case ROOK_PROMOTION:
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case KNIGHT_PROMOTION:
            board->pieces[WHITE_KNIGHTS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case BISHOP_PROMOTION:
            board->pieces[WHITE_BISHOPS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case QUEEN_PROMOTION:
            board->pieces[WHITE_QUEENS + color_shift] ^= MASK_SQUARE[move.target];
            break;
        case KING_CASTLE:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target + 1] | MASK_SQUARE[move.target - 1];
            board->pieces[board->current_color] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
            break;
        case QUEEN_CASTLE:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
            board->pieces[board->current_color] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
            break;
        default:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            break;
    }

    board->pieces[move.piece] ^= MASK_SQUARE[move.origin];
    board->pieces[board->current_color] ^= MASK_SQUARE[move.target];
    
    if (move.captured_piece != EMPTY)
    {
        board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target];
        board->pieces[!board->current_color] ^= MASK_SQUARE[move.target];
    }

    board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
    board->empty_squares = ~board->occupied_squares;
    board->available_squares = ~board->pieces[board->current_color];
    board->castle_permission = board->move_history[board->num_moves].castle_permission;
    board->en_passent = (board->move_history[board->num_moves].en_passent_target == -1) 
        ? 0
        : MASK_SQUARE[board->move_history[board->num_moves].en_passent_target];
}

/**
 * Appends each move in the move_mask to the given list.
 */
static inline void append_moves(ChessBoard *board, MoveList *list, BitBoard *move_mask, int origin, int piece)
{
    // Handles pawn promotions
    BitBoard promotion_mask = 0;
    if (piece == WHITE_PAWNS)
    {
        promotion_mask = *move_mask & MASK_RANK[RANK_8];
        *move_mask &= CLEAR_RANK[RANK_8];
    }
    else if (piece == BLACK_PAWNS)
    {
        promotion_mask = *move_mask & MASK_RANK[RANK_1];
        *move_mask &= CLEAR_RANK[RANK_1];
    }

    int target;
    while ((target = bitboard_pop(move_mask)) != -1)
    {
        int captured_piece = chessboard_get_piece(board, MASK_SQUARE[target]);

        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, NORMAL_MOVE
        };
    }
    while ((target = bitboard_pop(&promotion_mask)) != -1)
    {
        int captured_piece = chessboard_get_piece(board, MASK_SQUARE[target]);

        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, ROOK_PROMOTION
        };
        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, KNIGHT_PROMOTION
        };
        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, BISHOP_PROMOTION
        };
        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, QUEEN_PROMOTION
        };
    }
}

/**
 * Generates all possible moves in a given possition and adds them to the movelist provided.
 */
void chessboard_generate_moves(ChessBoard *board, MoveList *list)
{
    list->size = 0;

    BitBoard available_pawn_attacks;
    if (board->current_color == WHITE)
        available_pawn_attacks = board->pieces[BLACK] | board->en_passent;
    else
        available_pawn_attacks = board->pieces[WHITE] | board->en_passent;

    int start = (board->current_color == WHITE) ? WHITE_PAWNS : BLACK_PAWNS;
    for (int piece = start; piece < start + 6; piece++)
    {
        BitBoard attack_mask, piece_mask = board->pieces[piece];

        int origin;
        while((origin = bitboard_pop(&piece_mask)) != -1)
        {
            switch (piece)
            {
                case WHITE_PAWNS:
                    attack_mask = MASK_SQUARE[origin] << 8 & board->empty_squares;
                    attack_mask |= attack_mask << 8 & MASK_RANK[RANK_4] & board->empty_squares;
                    attack_mask |= MASK_PAWN_ATTACKS[WHITE][origin] & available_pawn_attacks;
                    break;
                case BLACK_PAWNS:
                    attack_mask = MASK_SQUARE[origin] >> 8 & board->empty_squares;
                    attack_mask |= attack_mask >> 8 & MASK_RANK[RANK_5] & board->empty_squares;
                    attack_mask |= MASK_PAWN_ATTACKS[BLACK][origin] & available_pawn_attacks;
                    break;
                case WHITE_ROOKS: case BLACK_ROOKS:
                    attack_mask = lookup_rook_attacks(origin, board->occupied_squares);
                    break;
                case WHITE_BISHOPS: case BLACK_BISHOPS:
                    attack_mask = lookup_bishop_attacks(origin, board->occupied_squares);
                    break;
                case WHITE_QUEENS: case BLACK_QUEENS:
                    attack_mask = lookup_queen_attacks(origin, board->occupied_squares);
                    break;
                case WHITE_KNIGHTS: case BLACK_KNIGHTS:
                    attack_mask = MASK_KNIGHT_ATTACKS[origin];
                    break;
                case WHITE_KING: case BLACK_KING:
                    attack_mask = MASK_KING_ATTACKS[origin];
                    break;
            }

            attack_mask &= board->available_squares;
            append_moves(board, list, &attack_mask, origin, piece);
        }
    }

    // Generates castling moves
    if (board->current_color == WHITE)
    {
        if (board->castle_permission | WHITE_KING_SIDE)
        {	
            bool passes_through_check = chessboard_squared_attacked(board, 4) 
                || chessboard_squared_attacked(board, 5) 
                || chessboard_squared_attacked(board, 6);
            bool path_clear = (board->empty_squares & MASK_F1_TO_G1) == MASK_F1_TO_G1;

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {4, 6, WHITE_KING, EMPTY, KING_CASTLE};
        }

        if (board->castle_permission | WHITE_QUEEN_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, 4) 
                || chessboard_squared_attacked(board, 3) 
                || chessboard_squared_attacked(board, 2);
            bool path_clear = (board->empty_squares & MASK_B1_TO_D1) == MASK_B1_TO_D1;

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {4, 2, WHITE_KING, EMPTY, KING_CASTLE};
        }
    }
    else
    {
        if (board->castle_permission | BLACK_KING_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, 60) 
                || chessboard_squared_attacked(board, 61) 
                || chessboard_squared_attacked(board, 62); 
            bool path_clear = (board->empty_squares & MASK_F8_TO_G8) == MASK_F8_TO_G8;

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {60, 62, BLACK_KING, EMPTY, KING_CASTLE};
        }

        if (board->castle_permission | BLACK_QUEEN_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, 60) 
                || chessboard_squared_attacked(board, 59) 
                || chessboard_squared_attacked(board, 58);
            bool path_clear = (board->empty_squares & MASK_B8_TO_D8) == MASK_B8_TO_D8; 

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {60, 58, BLACK_KING, EMPTY, KING_CASTLE};
        }
    }
}

/**
 * Prints a formated representation of a chessboard.
 */
void chessboard_print(ChessBoard *board)
{
    for (int rank = RANK_8; rank >= RANK_1; rank--)
    {
        printf("%i |", rank + 1);
        for (int file = FILE_A; file <= FILE_H; file++)
        {
            printf(" %c", piece_to_fen(chessboard_get_piece(board, FileRankToSquare(file, rank))));
        }
        printf("\n");
    }

    printf("   ----------------\n");
    printf("    A B C D E F G H\n");
}

Piece fen_to_piece(char f)
{
    switch (f) 
    {
        case 'P':
            return WHITE_PAWNS;
        case 'R':
            return WHITE_ROOKS;
        case 'N':
            return WHITE_KNIGHTS;
        case 'B':
            return WHITE_BISHOPS;
        case 'Q':
            return WHITE_QUEENS;
        case 'K':
            return WHITE_KING;
        case 'p':
            return BLACK_PAWNS;
        case 'r':
            return BLACK_ROOKS;
        case 'n':
            return BLACK_KNIGHTS;
        case 'b':
            return BLACK_BISHOPS;
        case 'q':
            return BLACK_QUEENS;
        case 'k':
            return BLACK_KING;
        default:
            return EMPTY;
    }
}

char piece_to_fen(Piece p)
{
    switch (p) 
    {
        case WHITE_PAWNS:
            return 'P';
        case WHITE_ROOKS:
            return 'R';
        case WHITE_KNIGHTS:
            return 'N';
        case WHITE_BISHOPS:
            return 'B';
        case WHITE_QUEENS:
            return 'Q';
        case WHITE_KING:
            return 'K';
        case BLACK_PAWNS:
            return 'p';
        case BLACK_ROOKS:
            return 'r';
        case BLACK_KNIGHTS:
            return 'n';
        case BLACK_BISHOPS:
            return 'b';
        case BLACK_QUEENS:
            return 'q';
        case BLACK_KING:
            return 'k';
        default:
            return '.';
    }
}
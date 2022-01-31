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
 * Initializes a chessboard's pieces with a fen stirng.
 **/
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

    board->position_key = chessboard_hash(board);
    board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
    board->empty_squares = ~board->occupied_squares;
}

/**
 * Returns a random U64 number.
 **/
static U64 generate_rand_number()
{
    U64 random_number = 0;
    random_number |= (U64) rand();
    random_number |= (U64) (rand() & 0xffff) << 16;
    random_number |= (U64) (rand() & 0xffff) << 32;
    random_number |= (U64) (rand() & 0xffff) << 48;

    return random_number;
}

static U64 PIECE_KEYS[12][64];
static U64 CASTLE_KEYS[16]; 
static U64 SIDE_KEY[2];

/**
 * Initializes the keys used to hash a chessboard.
 **/
void chessboard_init_keys(void)
{
    // Generates keys for PIECE_KEY
    for (int piece = 0; piece <  12; piece++)
    {
        for (int square = A1; square <= H8; square++)
        {
            PIECE_KEYS[piece][square] = generate_rand_number();
        }
    }


    // Generates keys for CASTLE_KEYS
    for (int castle_permission = 0; castle_permission <  16; castle_permission++)
    {
        CASTLE_KEYS[castle_permission] = generate_rand_number();
    }

    // Generates key for SIDE_KEY
    SIDE_KEY[WHITE] = generate_rand_number();
    SIDE_KEY[BLACK] = generate_rand_number();
}

/**
 * Returns a unique key based on the board's position.
 **/
U64 chessboard_hash(ChessBoard *board)
{   
    U64 hash = 0;
    for (int piece = WHITE_PAWNS; piece <= BLACK_KING; piece++)
    {
        BitBoard piece_bitboard = board->pieces[piece];

        int index;
        while ((index = bitboard_pop(&piece_bitboard)) != -1)
            hash ^= PIECE_KEYS[piece - 2][index];
    }

    hash ^= CASTLE_KEYS[board->castle_permission];
    hash ^= SIDE_KEY[board->current_color];

    return hash;
}

/**
 * Returns the piece on the given square.
 **/
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
 * Returns whether the given square is being attacked by an opponent
 * piece.
 **/
bool chessboard_squared_attacked(ChessBoard *board, int target)
{
    // Shift to change piece color from white to black
    int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

    // Checks if an opponent pawn, knight, king, bishop, rook, or queen can attack the given square
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
 * Updates the chessboard's pieces after a piece is moved.
 **/
void chessboard_move_piece(ChessBoard *board, Move move)
{
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
        case EN_PASSENT:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target + 8 * ((board->current_color == WHITE) ? -1 : 1)];
            board->pieces[!board->current_color] ^= MASK_SQUARE[move.target + 8 * ((board->current_color == WHITE) ? -1 : 1)];
            break;
        default:
            board->pieces[move.piece] ^= MASK_SQUARE[move.target];
            break;
    }

    board->pieces[move.piece] ^= MASK_SQUARE[move.origin];
    board->pieces[board->current_color] ^= MASK_SQUARE[move.origin] | MASK_SQUARE[move.target];
    
    if (move.captured_piece != EMPTY && move.move_type != EN_PASSENT)
    {
        board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target];
        board->pieces[!board->current_color] ^= MASK_SQUARE[move.target];
    }

    board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
    board->empty_squares = ~board->occupied_squares;
}


/**
 * Updates the chessboard's pieces after the given pseudo legal move is played. 
 * Returns whether the move is legal. If the move is not legal the board is not
 * updated.
 **/
bool chessboard_make_move(ChessBoard *board, Move move)
{
    board->move_history[board->num_moves].move = move;
    board->move_history[board->num_moves].position_key = board->position_key;
    board->move_history[board->num_moves].castle_permission = board->castle_permission;
    board->move_history[board->num_moves].en_passent_target = bitboard_pop(&board->en_passent);
    board->num_moves++;

    // Shift to change piece color from white to black
    int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

    chessboard_move_piece(board, move);

    // Invalid move if king is being attacked
    if (chessboard_squared_attacked(board, bitboard_scan_forward(board->pieces[WHITE_KING + color_shift])))
    {
        chessboard_undo_move(board);
        return false;
    }

    board->en_passent = 0;
    board->position_key = chessboard_hash(board);
    board->current_color = !board->current_color;

    // Removes castling permission if a king or rook moves
    switch (move.piece)
    {
        case WHITE_ROOKS: 
            if (move.origin == A1 || move.target == A1)
                board->castle_permission &= ~WHITE_QUEEN_SIDE;
            else if (move.origin == H1 || move.target == H1)
                board->castle_permission &= ~WHITE_KING_SIDE;
            break;
        case BLACK_ROOKS:
            if (move.origin == A8 || move.target == A8)
                board->castle_permission &= ~BLACK_QUEEN_SIDE;
            else if (move.origin == H8 || move.target == H8)
                board->castle_permission &= ~BLACK_KING_SIDE;
            break;
        case WHITE_KING:
            board->castle_permission &= ~WHITE_KING_SIDE;
            board->castle_permission &= ~WHITE_QUEEN_SIDE;
            break;
        case BLACK_KING:
            board->castle_permission &= ~BLACK_KING_SIDE;
            board->castle_permission &= ~BLACK_QUEEN_SIDE;
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
    
    // Removes castling rights if one of the rooks gets captured
    switch (move.captured_piece)
    {
        case WHITE_ROOKS: 
            if (move.target == A1)
                board->castle_permission &= ~WHITE_QUEEN_SIDE;
            else if (move.target == H1)
                board->castle_permission &= ~WHITE_KING_SIDE;
            break;
        case BLACK_ROOKS:
            if (move.target == A8)
                board->castle_permission &= ~BLACK_QUEEN_SIDE;
            else if (move.target == H8)
                board->castle_permission &= ~BLACK_KING_SIDE;
            break;
    }

    return true;
}

/**
 * Updates the chessboard's pieces after undoing the last moved played.
 **/
void chessboard_undo_move(ChessBoard *board)
{
    MoveInfo move_info = board->move_history[--board->num_moves];
    board->current_color = PieceColor(move_info.move.piece);

    chessboard_move_piece(board, move_info.move);

    board->position_key = move_info.position_key;
    board->castle_permission = move_info.castle_permission;
    board->en_passent = (move_info.en_passent_target != -1) 
        ? MASK_SQUARE[move_info.en_passent_target]
        : 0;
}

/**
 * Appends each move in the move_mask to the given MoveList.
 **/
static void append_moves(ChessBoard *board, MoveList *list, BitBoard *move_mask, int origin, int piece)
{
    // Handles pawn promotions
    BitBoard promotion_mask = 0, en_passent_mask = 0;
    if (piece == WHITE_PAWNS)
    {
        promotion_mask = *move_mask & MASK_RANK[RANK_8];
        *move_mask &= CLEAR_RANK[RANK_8];

        en_passent_mask = *move_mask & board->en_passent;
        *move_mask &= ~board->en_passent;
    }
    else if (piece == BLACK_PAWNS)
    {
        promotion_mask = *move_mask & MASK_RANK[RANK_1];
        *move_mask &= CLEAR_RANK[RANK_1];

        en_passent_mask = *move_mask & board->en_passent;
        *move_mask &= ~board->en_passent;
    }

    int target;
    while ((target = bitboard_pop(move_mask)) != -1)
    {
        int captured_piece = chessboard_get_piece(board, MASK_SQUARE[target]);

        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, NORMAL_MOVE
        };
    }
    while ((target = bitboard_pop(&en_passent_mask)) != -1)
    {
        int captured_piece = (piece == WHITE_PAWNS) ? BLACK_PAWNS : WHITE_PAWNS;

        list->moves[list->size++] = (Move) {
            origin, target, piece, captured_piece, EN_PASSENT
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
 * Generates all possible pseudo legal moves in a given possition 
 * and adds them to the given MoveList.
 **/
void chessboard_generate_moves(ChessBoard *board, MoveList *list)
{
    list->size = 0;

    // Gets squares pawns can attack on including en passent
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
                    // White pawn single push
                    attack_mask = MASK_SQUARE[origin] << 8 & board->empty_squares;
                    // White pawn double push
                    attack_mask |= attack_mask << 8 & MASK_RANK[RANK_4] & board->empty_squares;
                    // White pawn attack 
                    attack_mask |= MASK_PAWN_ATTACKS[WHITE][origin] & available_pawn_attacks;
                    break;
                case BLACK_PAWNS:
                    // Black pawn single push
                    attack_mask = MASK_SQUARE[origin] >> 8 & board->empty_squares;
                    // Black pawn double push
                    attack_mask |= attack_mask >> 8 & MASK_RANK[RANK_5] & board->empty_squares;
                    // Black pawn attack
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

            attack_mask &= ~board->pieces[board->current_color];

            append_moves(board, list, &attack_mask, origin, piece);
        }
    }

    // Generates castling moves
    if (board->current_color == WHITE)
    {   
        // White castle king side
        if (board->castle_permission & WHITE_KING_SIDE)
        {	
            bool passes_through_check = chessboard_squared_attacked(board, E1) 
                || chessboard_squared_attacked(board, F1) 
                || chessboard_squared_attacked(board, G1);
            bool path_clear = !(board->occupied_squares & MASK_F1_TO_G1);

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {E1, G1, WHITE_KING, EMPTY, KING_CASTLE};
        }

        // White castle queen side
        if (board->castle_permission & WHITE_QUEEN_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, E1) 
                || chessboard_squared_attacked(board, D1) 
                || chessboard_squared_attacked(board, C1);
            bool path_clear = !(board->occupied_squares & MASK_B1_TO_D1);

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {E1, C1, WHITE_KING, EMPTY, QUEEN_CASTLE};
        }
    }
    else
    {
        // Black castle king side
        if (board->castle_permission & BLACK_KING_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, E8) 
                || chessboard_squared_attacked(board, F8) 
                || chessboard_squared_attacked(board, G8); 
            bool path_clear = !(board->occupied_squares & MASK_F8_TO_G8);

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {E8, G8, BLACK_KING, EMPTY, KING_CASTLE};
        }

        // Black castle queen side
        if (board->castle_permission & BLACK_QUEEN_SIDE)
        {
            bool passes_through_check = chessboard_squared_attacked(board, E8) 
                || chessboard_squared_attacked(board, D8) 
                || chessboard_squared_attacked(board, C8);
            bool path_clear = !(board->occupied_squares & MASK_B8_TO_D8); 

            if (path_clear && !passes_through_check)
                list->moves[list->size++] = (Move) {E8, C8, BLACK_KING, EMPTY, QUEEN_CASTLE};
        }
    }
}

/**
 * Prints a formated representation of a chessboard.
 **/
void chessboard_print(ChessBoard *board)
{
    for (int rank = RANK_8; rank >= RANK_1; rank--)
    {
        printf("%i |", rank + 1);
        for (int file = FILE_A; file <= FILE_H; file++)
        {
            int piece = chessboard_get_piece(board, FileRankToSquare(file, rank));

            printf(" ");
            printf(piece_to_fen(piece));
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

char* piece_to_fen(Piece p)
{
    switch (p) 
    {
        case WHITE_PAWNS:
            return "P";
        case WHITE_ROOKS:
            return "R";
        case WHITE_KNIGHTS:
            return "N";
        case WHITE_BISHOPS:
            return "B";
        case WHITE_QUEENS:
            return "Q";
        case WHITE_KING:
            return "K";
        case BLACK_PAWNS:
            return "p";
        case BLACK_ROOKS:
            return "r";
        case BLACK_KNIGHTS:
            return "n";
        case BLACK_BISHOPS:
            return "b";
        case BLACK_QUEENS:
            return "q";
        case BLACK_KING:
            return "k";
        default:
            return ".";
    }
}
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

	if (MASK_PAWN_ATTACKS[board->current_color][target] & board->pieces[BLACK_PAWNS   - color_shift])
		return true;
	if (MASK_KNIGHT_ATTACKS[target] & board->pieces[BLACK_KNIGHTS - color_shift])
		return true;
	if (MASK_KING_ATTACKS[target] & board->pieces[BLACK_KING - color_shift])
		return true;
	if (lookup_bishop_attacks(target, board->occupied_squares) & (board->pieces[BLACK_QUEENS  - color_shift] | board->pieces[BLACK_BISHOPS - color_shift]))
		return true;
	if (lookup_rook_attacks(target, board->occupied_squares) & (board->pieces[BLACK_QUEENS  - color_shift] | board->pieces[BLACK_ROOKS - color_shift]))
		return true;

	return false;
}

/**
 * TODO: write description
 */
int chessboard_make_move(ChessBoard *board, Move move)
{
	board->history.moves[board->history.size++] = move;

	board->pieces[move.piece] ^= MASK_SQUARE[move.origin];

	// Updates captured piece information
	if (move.captured_piece != EMPTY)
		board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target];

	// Shift to change piece color from white to black
	int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

	// Updates the new position of the piece
	switch (move.flag)
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
			break;
		case QUEEN_CASTLE:
			board->pieces[move.piece] ^= MASK_SQUARE[move.target];
			board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
			break;
		default:
			board->pieces[move.piece] ^= MASK_SQUARE[move.target];
			break;
	}

	// Saves castling permission before updating incase the move needs to be undone
	int castling_permission = board->castle_permission;

	// Updates castling permissions
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
	}

	// Saves en passent target before updating incase the move needs to be undone
	int en_passent_target = (board->en_passent) ? bitboard_scan_forward(board->en_passent) : 0;

	// White pawn double push creates en passent square
	if (move.piece == WHITE_PAWNS && move.target - move.origin == 16)
	{
		board->en_passent = MASK_SQUARE[move.target - 8];
	}
	// Black pawn double push creates en passent square
	else if (move.piece == WHITE_PAWNS && move.target - move.origin == -16)
	{
		board->en_passent = MASK_SQUARE[move.target + 8];
	}
	else
	{
		board->en_passent = 0;
	}

	board->pieces[WHITE] = 0;
	board->pieces[BLACK] = 0;
	for (int i = 2; i < 14; i++)
	{
		if (PieceColor(i) == WHITE)
			board->pieces[WHITE] |= board->pieces[i];
		else
			board->pieces[BLACK] |= board->pieces[i];
	}

	board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
	board->empty_squares = ~board->occupied_squares;
	board->available_squares = ~board->pieces[board->current_color];
	
	if (chessboard_squared_attacked(board, bitboard_scan_forward(board->pieces[WHITE_KING + color_shift])))
	{
		board->current_color = !board->current_color;
		chessboard_undo_move(board, castling_permission, en_passent_target);
		return -1;
	}
	
	board->current_color = !board->current_color;
	return 1;
}

void chessboard_undo_move(ChessBoard *board, int castle_permission, int en_passent_target)
{
	Move move = board->history.moves[--board->history.size];
	board->current_color = !board->current_color;

	// Shift to change piece color from white to black
	int color_shift = (board->current_color == WHITE) ? 0 : (BLACK_PAWNS - WHITE_PAWNS); 

	switch (move.flag)
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
			break;
		case QUEEN_CASTLE:
			board->pieces[move.piece] ^= MASK_SQUARE[move.target];
			board->pieces[WHITE_ROOKS + color_shift] ^= MASK_SQUARE[move.target - 2] | MASK_SQUARE[move.target + 1];
			break;
		default:
			board->pieces[move.piece] ^= MASK_SQUARE[move.target];
			break;
	}

	board->pieces[move.piece] ^= MASK_SQUARE[move.origin];

	// Makes sure it captured a piece
	if (move.captured_piece != EMPTY)
		board->pieces[move.captured_piece] ^= MASK_SQUARE[move.target];

	board->pieces[WHITE] = 0;
	board->pieces[BLACK] = 0;
	for (int i = 2; i < 14; i++)
	{
		if (PieceColor(i) == WHITE)
			board->pieces[WHITE] |= board->pieces[i];
		else
			board->pieces[BLACK] |= board->pieces[i];
	}

	board->en_passent = MASK_SQUARE[en_passent_target];
	board->castle_permission = castle_permission;
	board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
	board->empty_squares = ~board->occupied_squares;
	board->available_squares = ~board->pieces[board->current_color];
}

/**
 * TODO: write description
 */
static inline void append_moves_by_direction(ChessBoard *board, MoveList *list, BitBoard move_mask, int shift, int piece, int flag)
{
	int index;
	while ((index = bitboard_pop(&move_mask)) != -1)
	{
		list->moves[list->size++] = (Move) {index - shift, index, piece, chessboard_get_piece(board, MASK_SQUARE[index]), flag};
	}
}

/**
 * TODO: write description
 */
static inline void append_moves_by_piece(ChessBoard *board, MoveList *list, BitBoard move_mask, int origin, int piece, int flag)
{
	int index;
	while ((index = bitboard_pop(&move_mask)) != -1)
	{
		list->moves[list->size++] = (Move) {origin, index, piece, chessboard_get_piece(board, MASK_SQUARE[index]), flag};
	}
}

/**
 * Generates all possible moves in a given possition and adds them to the movelist provided.
 */
void chessboard_generate_moves(ChessBoard *board, MoveList *list)
{
	list->size = 0;
	int pawns 	= (board->current_color == WHITE) ? WHITE_PAWNS 	: BLACK_PAWNS;
	int rooks 	= (board->current_color == WHITE) ? WHITE_ROOKS 	: BLACK_ROOKS;
	int king 	= (board->current_color == WHITE) ? WHITE_KING 		: BLACK_KING;
	int queens 	= (board->current_color == WHITE) ? WHITE_QUEENS 	: BLACK_QUEENS;
	int bishops = (board->current_color == WHITE) ? WHITE_BISHOPS 	: BLACK_BISHOPS;
	int knights = (board->current_color == WHITE) ? WHITE_KNIGHTS 	: BLACK_KNIGHTS;

	// Generates pawn pushes separately for white and black because their pawns move in opposite directions
	if (pawns == WHITE_PAWNS)
	{
		BitBoard single_pawn_push = board->pieces[WHITE_PAWNS] << 8 & board->empty_squares;
		append_moves_by_direction(board, list, single_pawn_push & CLEAR_RANK[RANK_8], 8, WHITE_PAWNS, 0);

		// Pawn promotions for each of the four possible pieces
		BitBoard single_pawn_push_promotions = single_pawn_push & MASK_RANK[RANK_8];
		append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, QUEEN_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, ROOK_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, BISHOP_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, KNIGHT_PROMOTION);

		// Doesn't need promotions for double pawn push because a double push can't reach the back rank
		BitBoard double_pawn_push = single_pawn_push << 8 & MASK_RANK[RANK_4] & board->empty_squares;
		append_moves_by_direction(board, list, double_pawn_push, 16, WHITE_PAWNS, 0);
	}
	else 
	{
		BitBoard single_pawn_push = board->pieces[BLACK_PAWNS] >> 8 & board->empty_squares;
		append_moves_by_direction(board, list, single_pawn_push & CLEAR_RANK[RANK_8], -8, BLACK_PAWNS, 0);

		// Pawn promotions for each of the four possible pieces
		BitBoard single_pawn_push_promotions = single_pawn_push & MASK_RANK[RANK_8];
		append_moves_by_direction(board, list, single_pawn_push_promotions, -8, BLACK_PAWNS, QUEEN_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, -8, BLACK_PAWNS, ROOK_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, -8, BLACK_PAWNS, BISHOP_PROMOTION);
		append_moves_by_direction(board, list, single_pawn_push_promotions, -8, BLACK_PAWNS, KNIGHT_PROMOTION);

		// Doesn't need promotions for double pawn push because a double push can't reach the back rank
		BitBoard double_pawn_push = single_pawn_push >> 8 & MASK_RANK[RANK_5] & board->empty_squares;
		append_moves_by_direction(board, list, double_pawn_push, -16, BLACK_PAWNS, 0);
	}

	// Generates pawn attacks using precalculated lookup table
	BitBoard pawn_attacks, pawn_attack_promotions;
	for (int index = bitboard_iter(&board->pieces[pawns]); index != -1; index = bitboard_iter(NULL))
	{
		pawn_attacks = MASK_PAWN_ATTACKS[board->current_color][index] & (board->pieces[!board->current_color] | board->en_passent);
		append_moves_by_piece(board, list, pawn_attacks & CLEAR_RANK[RANK_8], index, pawns, 0);

		// Pawn promotions for each of the four possible pieces
		pawn_attack_promotions = pawn_attacks & MASK_RANK[RANK_8];
		append_moves_by_piece(board, list, pawn_attack_promotions, index, pawns, QUEEN_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, pawns, ROOK_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, pawns, BISHOP_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, pawns, KNIGHT_PROMOTION);
	}

	// Generates knight attacks using precalculated lookup table
	BitBoard knight_attacks;
	for (int index = bitboard_iter(&board->pieces[knights]); index != -1; index = bitboard_iter(NULL))
	{
		knight_attacks = MASK_KNIGHT_ATTACKS[index] & board->available_squares;
		append_moves_by_piece(board, list, knight_attacks, index, knights, 0);
	}

	// Doesn't need loop to generate moves because their is only 1 king
	int king_index = bitboard_scan_forward(board->pieces[king]);
	BitBoard king_attacks = MASK_KING_ATTACKS[king_index] & board->available_squares;
	append_moves_by_piece(board, list, king_attacks, king_index, king, 0);

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
	
	// Queen moves are the combination of rook and bishops moves on the queen's square
	BitBoard queen_attacks;
	for (int index = bitboard_iter(&board->pieces[queens]); index != -1; index = bitboard_iter(NULL))
	{
		queen_attacks = lookup_queen_attacks(index, board->occupied_squares) & board->available_squares;
		append_moves_by_piece(board, list, queen_attacks, index, queens, 0);
	}

	// Generates bishop attacks using magic bitboards
	BitBoard bishop_attacks;
	for (int index = bitboard_iter(&board->pieces[bishops]); index != -1; index = bitboard_iter(NULL))
	{
		bishop_attacks = lookup_bishop_attacks(index, board->occupied_squares) & board->available_squares;
		append_moves_by_piece(board, list, bishop_attacks, index, bishops, 0);
	}

	// Generates rook attacks using magic bitboards
	BitBoard rook_attacks;
	for (int index = bitboard_iter(&board->pieces[rooks]); index != -1; index = bitboard_iter(NULL))
	{
		rook_attacks = lookup_rook_attacks(index, board->occupied_squares) & board->available_squares;
		append_moves_by_piece(board, list, rook_attacks, index, rooks, 0);
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
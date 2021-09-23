#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
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
				board->castle_permission |= BLACk_KING_SIDE;
				break;
			case 'q':
				board->castle_permission |= BLACK_QUEEN_SIDE;
				break;
		}
	}

	// Parses en passent square
	token = strtok(NULL, " ");
	if (token[0] != '-') 
		board->en_passent_target |= FileRankToSquare(token[0] - 'a', token[1] - '1');

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
Piece chessboard_get_piece(ChessBoard *board, BitBoard square) 
{
	if (board->empty_squares & square)
		return EMPTY;
	
	Piece start = (board->pieces[WHITE] & square) ? WHITE_PAWNS : BLACK_PAWNS;
	for (int piece = start; piece < start + 6; piece++) 
	{
		if (board->pieces[piece] & square)
			return piece;
	}

	return EMPTY;
}

typedef struct
{
	Move moves[256];
	U8 size;
} MoveList;

static void append_moves_by_direction(ChessBoard *board, MoveList *list, BitBoard move_mask, int shift, int piece, int flag)
{
	int target;
	while ((target = bitboard_pop(&move_mask)) != -1)
	{
		list->moves[list->size++] = (Move) {target - shift, target, piece, chessboard_get_piece(board, target), flag};
	}
}


static void append_moves_by_piece(ChessBoard *board, MoveList *list, BitBoard move_mask, int origin, int piece, int flag)
{
	int target;
	while ((target = bitboard_pop(&move_mask)) != -1)
	{
		list->moves[list->size++] = (Move) {origin, target, piece, chessboard_get_piece(board, target), flag};
	}
}

void chessboard_generate_moves(ChessBoard *board, MoveList *list)
{
	list->size = 0;

	// TODO generate moves for black pieces to because this function currently only generates moves for white pieces
	// TODO seperate pawn move generation into a seperate function for black and white for now just generate it for white
	BitBoard single_pawn_push = board->pieces[WHITE_PAWNS] << 8 & board->empty_squares;
	append_moves_by_direction(board, list, single_pawn_push & CLEAR_RANK[RANK_8], 8, WHITE_PAWNS, 0);

	BitBoard single_pawn_push_promotions = single_pawn_push & MASK_RANK[RANK_8];
	append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, QUEEN_PROMOTION);
	append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, ROOK_PROMOTION);
	append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, BISHOP_PROMOTION);
	append_moves_by_direction(board, list, single_pawn_push_promotions, 8, WHITE_PAWNS, KNIGHT_PROMOTION);

	BitBoard double_pawn_push = single_pawn_push << 8 & MASK_RANK[RANK_4] & board->empty_squares;
	append_moves_by_direction(board, list, double_pawn_push, 8, WHITE_PAWNS, 0);

	BitBoard pawn_attacks, pawn_attack_promotions;
	for (int index = bitboard_iter(board->pieces[WHITE_PAWNS]); index != -1; index = bitboard_iter(NULL))
	{
		pawn_attacks = MASK_PAWN_ATTACKS[WHITE][index] & (board->pieces[BLACK] | board->en_passent_target);
		append_moves_by_piece(board, list, pawn_attacks & CLEAR_RANK[RANK_8], index, WHITE_PAWNS, 0);

		pawn_attack_promotions = pawn_attacks & MASK_RANK[RANK_8];
		append_moves_by_piece(board, list, pawn_attack_promotions, index, WHITE_PAWNS, QUEEN_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, WHITE_PAWNS, ROOK_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, WHITE_PAWNS, BISHOP_PROMOTION);
		append_moves_by_piece(board, list, pawn_attack_promotions, index, WHITE_PAWNS, KNIGHT_PROMOTION);
	}

	BitBoard knight_attacks;
	for (int index = bitboard_iter(board->pieces[WHITE_KNIGHTS]); index != -1; index = bitboard_iter(NULL))
	{
		knight_attacks = MASK_KNIGHT_ATTACKS[index] & board->available_squares;
		append_moves_by_piece(board, list, knight_attacks, index, WHITE_KNIGHTS, 0);
	}

	int king_index = bitboard_scan_forward(board->pieces[WHITE_KING]);
	BitBoard king_attacks = MASK_KING_ATTACKS[king_index] & board->available_squares;
	append_moves_by_piece(board, list, king_attacks, king_index, WHITE_KING, 0);

	// Generates castling moves
	if (board->castle_permission | WHITE_KING_SIDE && board->empty_squares & 0x60)
		list->moves[list->size++] = (Move) {4, 6, WHITE_KING, EMPTY, KING_CASTLE};

	if (board->castle_permission | WHITE_QUEEN_SIDE && board->empty_squares & 0xe)
		list->moves[list->size++] = (Move) {2, 6, WHITE_KING, EMPTY, KING_CASTLE};
	
	int queen_index = bitboard_scan_forward(board->pieces[WHITE_QUEEN]);
	BitBoard queen_attacks = (lookup_bishop_attacks(queen_index, board->occupied_squares) | lookup_rook_attacks(queen_index, board->occupied_squares)) & board->available_squares;
	append_moves_by_piece(board, list, queen_attacks, queen_index, WHITE_QUEEN, 0);

	BitBoard bishop_attacks;
	for (int index = bitboard_iter(board->pieces[WHITE_BISHOPS]); index != -1; index = bitboard_iter(NULL))
	{
		bishop_attacks = lookup_bishop_attacks(index, board->occupied_squares) & board->available_squares;
		append_moves_by_piece(board, list, bishop_attacks, index, WHITE_BISHOPS, 0);
	}

	BitBoard rook_attacks;
	for (int index = bitboard_iter(board->pieces[WHITE_ROOKS]); index != -1; index = bitboard_iter(NULL))
	{
		rook_attacks = lookup_rook_attacks(index, board->occupied_squares) & board->available_squares;
		append_moves_by_piece(board, list, rook_attacks, index, WHITE_ROOKS, 0);
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
			return WHITE_QUEEN;
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
			return BLACK_QUEEN;
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
		case WHITE_QUEEN:
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
		case BLACK_QUEEN:
			return 'q';
		case BLACK_KING:
			return 'k';
		default:
			return '.';
	}
}
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "chessboard.h"
#include "lookup_tables.h"

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

// TODO: make a function for adding the things to a generated move list

/**
 * Generates all pseudo legal moves for black pawns.
 */
void chessboard_generate_black_pawn_moves(ChessBoard *board, GeneratedMoves *m)
{
	BitIndices bits;

	// Single black pawn push
	BitBoard target_squares = (board->pieces[BLACK_PAWNS] >> 8) & board->empty_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 8;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = BLACK_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Double black pawn push
	target_squares = (board->pieces[BLACK_PAWNS] >> 8) & board->empty_squares;
	target_squares = (target_squares >> 8) & board->empty_squares & MASK_RANK[RANK_5];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 16;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = BLACK_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// East black pawn attack
	target_squares = (board->pieces[WHITE] | board->en_passent_target) & (board->pieces[BLACK_PAWNS] >> 7) & CLEAR_FILE[FILE_A];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 7;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = BLACK_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// West black pawn attack
	target_squares = (board->pieces[WHITE] | board->en_passent_target) & (board->pieces[BLACK_PAWNS] >> 9) & CLEAR_FILE[FILE_H];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 9;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = BLACK_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}
}

/**
 * Generates all pseudo legal moves for white pawns.
 */
void chessboard_generate_white_pawn_moves(ChessBoard *board, GeneratedMoves *m)
{
	BitIndices bits;

	// Single white pawn push
	BitBoard target_squares = (board->pieces[WHITE_PAWNS] << 8) & board->empty_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 8;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = WHITE_PAWNS;
		m->moves[m->size].captured_piece = EMPTY;
		m->moves[m->size].flags = 0;
	}

	// Double white pawn push
	target_squares = (board->pieces[WHITE_PAWNS] << 8) & board->empty_squares;
	target_squares = (target_squares << 8) & board->empty_squares & MASK_RANK[RANK_4];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 16;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = WHITE_PAWNS;
		m->moves[m->size].captured_piece = EMPTY;
		m->moves[m->size].flags = 0;
	}

	// East white pawn attack (/)
	target_squares = (board->pieces[WHITE] | board->en_passent_target) & (board->pieces[WHITE_PAWNS] << 9) & CLEAR_FILE[FILE_A];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 9;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = WHITE_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// West white pawn attack (\)
	target_squares = (board->pieces[WHITE] | board->en_passent_target) & (board->pieces[WHITE_PAWNS] << 7) & CLEAR_FILE[FILE_H];
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 7;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = WHITE_PAWNS;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// TODO: add moves for promotion
}

/**
 * Generates all pseudo legal moves for knights.
 */
void chessboard_generate_knight_moves(ChessBoard *board, GeneratedMoves *m)
{
	BitIndices bits;
	int knights = (board->current_color == WHITE) ? WHITE_KNIGHTS : BLACK_KNIGHTS;

	// Knight attack north north east
	BitBoard target_squares = (board->pieces[knights] << 17) & CLEAR_FILE[FILE_A] & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 17;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack north east east
	target_squares = (board->pieces[knights] << 10) & CLEAR_FILE_AB & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 10;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack north north west
	target_squares = (board->pieces[knights] << 15) & CLEAR_FILE[FILE_H] & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 15;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack north west west
	target_squares = (board->pieces[knights] << 6) & CLEAR_FILE_GH & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] - 6;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack south east east
	target_squares = (board->pieces[knights] >> 6) & CLEAR_FILE_AB & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 6;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack south south east
	target_squares = (board->pieces[knights] >> 15) & CLEAR_FILE[FILE_A] & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 15;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}


	// Knight attack south west west
	target_squares = (board->pieces[knights] >> 10) & CLEAR_FILE_GH & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 10;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// Knight attack south south west
	target_squares = (board->pieces[knights] >> 17) & CLEAR_FILE[FILE_H] & board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = bits.indices[i] + 17;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = knights;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}
}

/**
 * Generates all pseudo legal moves for the king.
 */
void chessboard_generate_king_moves(ChessBoard *board, GeneratedMoves *m)
{
	BitIndices bits;
	int king = (board->current_color == WHITE) ? WHITE_KING : BLACK_KING;
	int king_square = bitboard_scan_forward(board->pieces[king]);

	// King attacks
	BitBoard target_squares = ((board->pieces[king] << 1) & CLEAR_FILE[FILE_A]) | ((board->pieces[king] >> 1) & CLEAR_FILE[FILE_H]);
	target_squares |= ((board->pieces[king] | target_squares) << 8) | ((board->pieces[king] | target_squares) >> 8);
	target_squares &= board->available_squares;
	bitboard_index(&bits, target_squares);
	for (int i = 0; i < bits.size; i++, m->size++)
	{
		m->moves[m->size].origin = king_square;
		m->moves[m->size].target = bits.indices[i];
		m->moves[m->size].piece = king;
		m->moves[m->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i]]);
		m->moves[m->size].flags = 0;
	}

	// TODO: implement castling functionality
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
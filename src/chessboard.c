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
	// TODO: make a separate fen_str to copy into so the original does not get modified

	// Clears board
	memset(board, 0, sizeof(ChessBoard));

	// Parses pieces in fen string
	char *token = strtok(fen_str, " ");
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
}

/**
 * Returns the piece on the given square.
 */
Piece chessboard_get_piece(ChessBoard *board, Bitboard square) 
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

Bitboard wSinglePushTargets(ChessBoard *board) {
   return (board->pieces[WHITE_PAWNS] << 8) & board->empty_squares;
}

Bitboard wDoublePushTargets(ChessBoard *board) {
   Bitboard single_pushs = wSinglePushTargets(board);
   return (single_pushs << 8) & board->empty_squares & MASK_RANK[RANK_4];
}

Bitboard bSinglePushTargets(ChessBoard *board) {
   return (board->pieces[BLACK_PAWNS] >> 8) & board->empty_squares;
}

Bitboard bDoublePushTargets(ChessBoard *board) {
   Bitboard single_pushs = bSinglePushTargets(board);
   return (single_pushs >> 8) & board->empty_squares & MASK_RANK[RANK_5];
}

void chessboard_generate_pawn_moves(ChessBoard *board, GeneratedMoves *moves)
{
	BitIndices bits;
	bool is_white = board->current_color == WHITE;

	// Generates moves for a pawn single push
	Bitboard single_push_targets = (is_white) ? wSinglePushTargets(board) : bSinglePushTargets(board);
	bitboard_index(&bits, single_push_targets);
	for (int i = 0; i < bits.size; i++)
	{
		moves->moves[moves->size].origin = bits.indices[i] + (is_white ? -8 : 8);
		moves->moves[moves->size].target = bits.indices[i];
		moves->moves[moves->size].piece = (is_white ? WHITE_PAWNS : BLACK_PAWNS);
		moves->moves[moves->size].captured_piece = EMPTY;
		moves->moves[moves->size++].flags = 0;
	}

	// Generates moves for a pawn double push
	Bitboard double_push_targets = (is_white) ? wDoublePushTargets(board) : bDoublePushTargets(board);
	bitboard_index(&bits, double_push_targets);
	for (int i = 0; i < bits.size; i++)
	{
		moves->moves[moves->size].origin = bits.indices[i] + (is_white ? -16 : 16);
		moves->moves[moves->size].target = bits.indices[i];
		moves->moves[moves->size].piece = (is_white ? WHITE_PAWNS : BLACK_PAWNS);
		moves->moves[moves->size].captured_piece = EMPTY;
		moves->moves[moves->size++].flags = 0;
	}

	// Generates moves for a pawn attack
	bitboard_index(&bits, board->pieces[(is_white ? WHITE_PAWNS : BLACK_PAWNS)]);
	for (int i = 0; i < bits.size; i++) 
	{
		// Attack to the left
		if ((board->pieces[!board->current_color] | board->en_passent_target) & MASK_SQUARE[bits.indices[i] + (is_white ? 7 : -9)] & CLEAR_FILE[FILE_H])
		{
			moves->moves[moves->size].origin = bits.indices[i];
			moves->moves[moves->size].target = bits.indices[i] + (is_white ? 7 : -9);
			moves->moves[moves->size].piece = (is_white ? WHITE_PAWNS : BLACK_PAWNS);
			moves->moves[moves->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i] + (is_white ? 7 : -9)]);
			moves->moves[moves->size++].flags = (board->en_passent_target & MASK_SQUARE[bits.indices[i] + (is_white ? 7 : -9)]) ? EN_PASSANT : 0;
		}

		// Attack to the right
		if ((board->pieces[!board->current_color] | board->en_passent_target) & MASK_SQUARE[bits.indices[i] + (is_white ? 9 : -7)] & CLEAR_FILE[FILE_A])
		{
			moves->moves[moves->size].origin = bits.indices[i];
			moves->moves[moves->size].target = bits.indices[i] + (is_white ? 9 : -7);
			moves->moves[moves->size].piece = (is_white ? WHITE_PAWNS : BLACK_PAWNS);
			moves->moves[moves->size].captured_piece = chessboard_get_piece(board, MASK_SQUARE[bits.indices[i] + (is_white ? 9 : -7)]);
			moves->moves[moves->size++].flags = (board->en_passent_target & MASK_SQUARE[bits.indices[i] + (is_white ? 9 : -7)]) ? EN_PASSANT : 0;
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
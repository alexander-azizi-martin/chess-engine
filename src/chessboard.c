#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "chessboard.h"

/**
 * Initializes a chessboard's position with a fen stirng.
 */
void chessboard_init(ChessBoard *board, string fen_str)
{
	// Initializes board
	memset(board, 0, sizeof(ChessBoard));
	Bitboard index = (U64)1 << 56;

	// Parses fen string
	for (int i = 0, row = 0; fen_str[i] != '\0' || fen_str[i] == ' '; i++)
	{
		if (fen_str[i] == '/')
		{
			// Goes to the start of the next row
			index = ((U64)1 << 56) >> (8 * ++row);
		}
		else if (isdigit(fen_str[i]))
		{
			// Shifts the index by number of empty spaces
			index <<= fen_str[i] - 48;
		}
		else
		{
			// Places the piece on the board
			int piece_color = isupper(fen_str[i]) ? WHITE : BLACK; 
			int piece_type = fen_to_piece(fen_str[i]);

			board->pieces[piece_type] |= index;
			board->pieces[piece_color] |= index;
			index <<= 1;
		}
	}

	board->occupied_squares = board->pieces[WHITE] | board->pieces[BLACK];
	board->empty_squares = ~board->occupied_squares;
}

/**
 * Returns the piece on the given square.
 */
Piece chessboard_get_piece(ChessBoard *board, Bitboard index) 
{
	if (board->empty_squares & index)
		return EMPTY;
	
	Piece start = (board->pieces[WHITE] & index) ? WHITE_PAWNS : BLACK_PAWNS;
	for (int piece = start; piece < start + 6; piece++) 
	{
		if (board->pieces[piece] & index)
			return piece;
	}

	return EMPTY;
}

/**
 * Prints a formated representation of a chessboard.
 */
void chessboard_print(ChessBoard *board)
{
	char buffer[21];
	Bitboard index;

	for (int rank = 0; rank < 8; rank++)
	{
		snprintf(buffer, 4, "%i |", 8 - rank);

		for (int file = 0; file < 8; file++)
		{
			// Calculates the square index
			index = (U64)1 << ((7 - rank) * 8 + file);

			// Sets the piece on the board;
			buffer[(3 + file * 2)] = ' '; 
			buffer[(3 + file * 2) + 1] = piece_to_fen(chessboard_get_piece(board, index));
		}

		buffer[19] = '\n';
		buffer[20] = '\0';
		printf(buffer);
	}

	strcpy(buffer, "   ----------------\n");
	printf(buffer);

	strcpy(buffer, "    A B C D E F G H\n");
	printf(buffer);
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
		return '*';
	}
}
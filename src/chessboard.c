#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "chessboard.h"

static PieceType fen_to_piece(char c) 
{
	switch(tolower(c)) 
	{
	case 'p':
		return PAWN;
	case 'r':
		return ROOK;
	case 'n':
		return KNIGHT;
	case 'b':
		return BISHOP;
	case 'q':
		return QUEEN;
	case 'k':
		return KING;
	default:
		return EMPTY;
	}
}

static char piece_to_fen(PieceType p)
{
	switch(p) 
	{
	case PAWN:
		return 'p';
	case ROOK:
		return 'r';
	case KNIGHT:
		return 'n';
	case BISHOP:
		return 'b';
	case QUEEN:
		return 'q';
	case KING:
		return 'k';
	default:
		return '*';
	}
}

ChessBoard *chessboard_init(string fen_str)
{
	// Initializes board
	ChessBoard *board = malloc(sizeof(ChessBoard));
	memset(board, 0, sizeof(ChessBoard));

	// Parses fen string
	Bitboard position = ((U64)1 << 56);
	for (int i = 0, row = 0; fen_str[i] != '\0' || fen_str[i] == ' '; i++)
	{
		if (fen_str[i] == '/')
		{
			// Goes to the start of the next row
			position = ((U64)1 << 56) >> (8 * ++row);
		}
		else if (isdigit(fen_str[i]))
		{
			// Shifts position by number of empty spaces
			position <<= fen_str[i] - 48;
		}
		else
		{
			// Places the piece on the board
			int piece_color = isupper(fen_str[i]) ? WHITE : BLACK; 
			int piece_type = fen_to_piece(fen_str[i]);

			board->pieces[piece_color][piece_type] ^= position;
			position <<= 1;
		}
	}

	return board;
}

void chessboard_print(ChessBoard *board)
{
	char buffer[21] = ' ';
	int buffer_position;
	U64 position;


	for (int row = 0; row < 8; row++)
	{
		snprintf(buffer, 4, "%i |", 8 - row);

		for (int col = 0; col < 8; col++)
		{
			buffer_position = (3 + col * 2) + 1;
			buffer[buffer_position - 1] = ' ';
			position = (U64)1 << ((7 - row) * 8 + col);

			for (int color = 0; color < 2; color++) {
				for (int piece = 0; piece < 7; piece++){
					if (board->pieces[color][piece] & position)
						buffer[buffer_position] = (color == WHITE) ? toupper(piece_to_fen(piece)) : piece_to_fen(piece);
				}
			}
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
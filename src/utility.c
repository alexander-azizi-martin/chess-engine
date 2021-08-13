#include <string.h>
#include <stdio.h>
#include "utility.h"

void print_bitboard(Bitboard board)
{
	char buffer[21];

	for (int row = 0; row < 8; row++)
	{
		snprintf(buffer, 4, "%i |", 8 - row);

		for (int col = 0; col < 8; col++)
		{
			Bitboard position = (U64)1 << ((7 - row) * 8 + col);

			buffer[3 + col * 2] = ' ';
			if (board & position)
				buffer[(3 + col * 2) + 1] = '1';
			else
				buffer[(3 + col * 2) + 1] = '0';
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

#include <stdio.h>
#include <string.h>
#include "magic_bitboard.h"
#include "lookup_tables.h"
#include "chessboard.h"
#include "test.h"

U64 count_moves(ChessBoard *board, int depth)
{
    if (depth == 0)
        return 1;

    U64 total_moves = 0;

    MoveList list;
    chessboard_generate_moves(board, &list);
    for (int i = 0; i < list.size; i++)
    {
        if (chessboard_make_move(board, list.moves[i]))
        {
            total_moves += count_moves(board, depth - 1);

            chessboard_undo_move(board);
        }
    }

    return total_moves;
}

int main()
{
    magic_bitboards_init();
    lookup_tables_init();

    FILE *file_ptr = fopen("perftsuite.epd", "r");
    if (file_ptr == NULL)
    {
        printf_error("Could not open testing file.\n");
        return 1;
    }
    int i = 0;
    char line[500];
    while (fgets(line, sizeof(line), file_ptr) != NULL)
    {
        char fen_str[101];
        sscanf(line, "%100[^;]", fen_str);

        printf("%s\n", fen_str);

        ChessBoard board;
        chessboard_init(&board, fen_str);

        char *token = strtok(line, ";");
        while((token = strtok(NULL, ";")) != NULL)
        {
            U64 depth, expected_num_moves;
            sscanf(token, "D%I64i %I64i", &depth, &expected_num_moves);

            if (i == 1 && depth == 6)
                continue;

            U64 generated_num_moves = count_moves(&board, depth);
            if (generated_num_moves == expected_num_moves)
                printf_success("  Correct number of moves generated at depth %i\n", depth);
            else
            {    
                printf_error("  Expected %I64i number of moves but generated %I64i\n", expected_num_moves, generated_num_moves);
                break;
            }
        }
        i++;
    }

    if (ferror(file_ptr))
    {
        printf_error("Error occurred while reading file.\n");
    }

    fclose(file_ptr);
    return 0;
}
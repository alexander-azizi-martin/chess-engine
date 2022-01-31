#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <stdio.h>
#include "magic_bitboard.h"
#include "lookup_tables.h"
#include "chessboard.h"

#define TESTING_DEPTH 4

typedef struct
{
    char fen_str[101];
    U64 correct_num_moves[6];
} TestMoveParameters;

/**
 * Returns a dynamically allocated list of paramaters parsed from the given file 
 * containing information on the correct number of legal moves up to a depth of 6.
 * 
 * @note The num_tests paramater is filled with the size of the allocated list.
 */
TestMoveParameters *parse_move_data(char *filename, int *num_tests);

/**
 * Free's the parameters allocated for move generation tests.
 */
void free_move_data(struct criterion_test_params *parameters);

/**
 * Initializes all the lookup tables used for move generation and
 * board hasing.
 */
void init_all(void);

/**
 * Returns the number of possible legal moves that can be played
 * up to a certain depth.
 */
U64 count_moves(ChessBoard *board, int depth);

/**
 * Tests move generation by checking that the correct number of moves is
 * generated for various positions up a depth of TEST_DEPTH.
 */
ParameterizedTestParameters(chess_board, move_generation)
{
    int num_tests;
    TestMoveParameters *test_data = parse_move_data("tests/data/perftsuite.epd", &num_tests);

    return cr_make_param_array(TestMoveParameters, test_data, num_tests, free_move_data);
}

ParameterizedTest(TestMoveParameters *test, chess_board, move_generation, .init = init_all)
{
    ChessBoard board;
    chessboard_init(&board, test->fen_str);

    for (int depth = 1; depth <= TESTING_DEPTH; depth++)
    {
        U64 calculated_num_moves = count_moves(&board, depth);

        cr_assert_eq(calculated_num_moves, test->correct_num_moves[depth - 1]);
    }
}

TestMoveParameters *parse_move_data(char *filename, int *num_tests)
{
    FILE *file_ptr = fopen(filename, "r");
    if (file_ptr == NULL)
    {
        printf("Could not open testing data.\n");
        exit(1);
    }

    int num_lines = 0, character;
    while ((character = fgetc(file_ptr)) != EOF)
    {
        if (character == '\n')
            num_lines++;
    }
    rewind(file_ptr);

    TestMoveParameters *test_data = cr_malloc(num_lines * sizeof(TestMoveParameters));
    *num_tests = num_lines;
    if (test_data == NULL)
    {
        printf("Could not allocate testing data.\n");
        exit(1);
    }

    char line[1000];
    for (int i = 0; fgets(line, sizeof(line), file_ptr) != NULL; i++)
    {
        char *token = strtok(line, ";");

        if (sizeof(test_data->fen_str) < strlen(token))
        {
            printf("Testing data contains malformated fen-string.\n");
            exit(1);
        }

        strcpy(test_data[i].fen_str, token);

        while ((token = strtok(NULL, ";")) != NULL)
        {
            int depth;
            U64 correct_num_moves;

            sscanf(token, "D%i %llu", &depth, &correct_num_moves);

            if (!(0 <= depth && depth <= sizeof(test_data->correct_num_moves)))
            {
                printf("Testing data contains a depth which exceeds 6.\n");
                exit(1);
            }

            test_data[i].correct_num_moves[depth - 1] = correct_num_moves;
        }
    }

    fclose(file_ptr);

    return test_data;
}

void free_move_data(struct criterion_test_params *parameters)
{
    cr_free(parameters->params);
}

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

void init_all(void)
{
    chessboard_init_keys();
    magic_bitboards_init();
    lookup_tables_init();
}
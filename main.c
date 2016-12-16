/*
 *  pgn2fen - Extracts FEN of a specific move on a PGN game
 *  -------------------------------------------------------
 *  Author: mate_amargo
 *  https://github.com/mate-amargo/pgn2fen
 *  -------------------------------------------------------
 *
 *  Usage:
 *  pgn2fen input_game.pgn move [w/b] [output_position.fen]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NARGS 2			/* Mandatory arguments */
#define NARGSOPT 2	/* Optional arguments */
#define FILES 8			/* Board files (columns) (a-h) */
#define RANKS 8			/* Board ranks (rows) (1-8) */

int main (int argc, char **argv) {

	int move, i;
	char side = 'w';
	FILE 	*finput, *foutput;

	/* Check the program arguments */
	if ((argc-1 >= NARGS) && (argc-1 <= NARGS + NARGSOPT)) {
		if ((finput = fopen(argv[1], "r")) == NULL) {
			printf("*** Error: The input file \"%s\" could not be opened\n", argv[1]);
			exit(EXIT_FAILURE);				
		} else if ((move = atoi(argv[2])) <= 0) {
			fclose(finput);
			printf("*** Error: Invalid move number \"%s\"\n", argv[2]);
			exit(EXIT_FAILURE);
		} else if (argc-1 > NARGS) { /* Optional arguments */
			if (3 == argc-1) {
				if (strlen(argv[3]) == 1) {
					side = tolower(argv[3][0]);
					if (side != 'w' && side != 'b') {
						fclose(finput);
						printf("*** Error: Invalid side \"%s\"\n", argv[3]);
						exit(EXIT_FAILURE);
					}
				} else if ((foutput = fopen(argv[3], "w")) == NULL) {
						printf("*** Error: The output file \"%s\" could not be opened\n", argv[3]);
						exit(EXIT_FAILURE);				
				}
			} else { /* Four arguments */
				if (strlen(argv[3]) == 1) {
					side = tolower(argv[3][0]);
					if (side != 'w' && side != 'b') {
						fclose(finput);
						printf("*** Error: Invalid side \"%s\"\n", argv[3]);
						exit(EXIT_FAILURE);
					}
				} else {
						fclose(finput);
						printf("*** Error: Invalid side \"%s\"\n", argv[3]);
						exit(EXIT_FAILURE);
				}
				if ((foutput = fopen(argv[4], "w")) == NULL) {
						printf("*** Error: The output file \"%s\" could not be opened\n", argv[4]);
						exit(EXIT_FAILURE);				
				}

			}
		}
	} else {
		printf("Usage: %s input_game.pgn move [w/b] [output_position.fen]\n", argv[0]);
		printf("  input_game.pgn       - A chess game in PGN format.\n");
		printf("  move                 - A move number.\n");
		printf("  w/b                  - OPTIONAL. Position reached after (w)hite or (b)lack move. Defaults to w.\n");
		printf("  output_position.fen  - OPTIONAL. Output file. If not specified the output will be written to stdout.\n");
		printf("\n\nFor example, if game.png contains:\n");
		printf("1. e4 c5 2. Nf3 d6\n");
		printf("To print the position after white's second move:\n");
		printf("%s game.pgn 2\n", argv[0]);
		printf("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2\n");
		printf("To print the position after black's second move:\n");
		printf("%s game.pgn 2 b\n", argv[0]);
		printf("rnbqkbnr/pp2pppp/3p4/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3\n");
		exit(EXIT_FAILURE);
	}
	/* Everything is ok, now let's work: */
	

	exit(EXIT_SUCCESS);

}

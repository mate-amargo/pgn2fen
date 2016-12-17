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

	int move /* move number argument */;
	char side = 'w'; /* Default side is white */
	FILE 	*finput, *foutput = NULL;

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
	
	if (foutput == NULL) /* They didn't specify an output file so write to stdout */
		foutput = stdout;
	
	/* List to hold the moves */
	struct tlist {
		char move[8]; /* Max length is 6, e.g. Nbxf3+, O-O-O+; 5 if we don't count checks/mates */
		struct tlist *next;
	};

	struct tlist *list, *x, *y; /* List head pointed by "list", "x" & "y" are auxiliary */
	list = malloc(sizeof(struct tlist)); /* Create an empty list member to make life easier */
	list->next = NULL;
	x = list;

	int i; /* Why didn't we have one of these before?? */
	char c, d;	/* Buffers to hold chars */
	int breakout = 0, breakout2 = 0; /* flags to breakout of while loops from within switch statements */
	int ply = 1; /* "ply" is the half-move number in the pgn file. "move" is the argument. */
	int blackmove = 0; /* If we want the position after black moved, we have to go one more loop iteration */
	if ('b' == side)
		blackmove = 1;
	/* Load the moves into the linked list */
	while (!feof(finput) && ((ply+blackmove)/2 < move + blackmove)) {
		/* Add a new item to the list */
		y = malloc(sizeof(struct tlist));
		y->next = NULL;

		i = 0; /* Set's the position of the move array (of chars) to 0 */
		breakout = 0;
		while (!breakout && (c = fgetc(finput)) != EOF) {
			switch (c) {
				case '[': /* It's a tag, read past it */
					while (fgetc(finput) != '\n');
					break;
				case '(': /* Variation, read past it, after the space */
					while (fgetc(finput) != ')');
					fgetc(finput); /* Discard space */
					break;
				case '{': /* Commentary, read past it, after the space */
					while (fgetc(finput) != '}');
					fgetc(finput); /* Discard space */
					break;
				case '1':	case '2':	case '3':	case '4':	case '5':	case '6':	case '7':	case '8': case '9':
					breakout2 = 0;
					/* Distinguish between move numbers and things like e4xd5 */
					d = c; /* Keep the old number in hand */
					while (!breakout2 && (c = fgetc(finput)) != EOF) { /* If it's a number keep eating chars */
						switch (c) {
							case '.': /* It's the beginning of a move, so eat the space */
								fgetc(finput);
								breakout2 = 1;
								break;
							/* It's not a number, so store the first number and the following char, and break out the loop */
							case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':
							case 'R':	case 'N':	case 'B': case 'Q': case 'K':
							case 'x': case 'O': case '-':
								y->move[i++] = d;
								y->move[i++] = c;
								breakout = breakout2 = 1;
								break;
							case ' ': /* It's the ending of a move with number like e4 */
								y->move[i++] = d;
								breakout = breakout2 = 1;
								break;
						}
					}
					if (feof(finput)) { /* We reached EOF while saving the last move! */
						y->move[i++] = d; 
						breakout = 1; 
						clearerr(finput); /* Clear the EOF flag */
					}
					break;
				case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':
				case 'R':	case 'N':	case 'B': case 'Q': case 'K':
				case 'x': case 'O': case '-':
					y->move[i++] = c;
					break;
				case ' ': /* Finish reading the move. Break out of the loop in a cheaty way! */
					breakout = 1;
					break;
			}
		} 
		x->next = y;
		x = y;
		ply++;
	}

	if(feof(finput)) {
		printf("*** Error: Move number %d by %s does not exist\n", move, (side == 'w')?"white":"black");
		exit(EXIT_FAILURE);
	}

	x = list;
	while (x) { printf("%s ", x->move); x = x->next; } printf("\n");

	exit(EXIT_SUCCESS);

}

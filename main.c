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
#define K (1 << 3)	/* White can castle Kingside */
#define Q (1 << 2)	/* White can castle Queenside */
#define k (1 << 1)	/* Black can castle Kingside */
#define q 1 				/* Black can castle Queenside */
#define WHITE 1			/* Used to determine which turn... */
#define BLACK 0			/* ...is whilst traversing the list of moves */

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

	int i, j; /* Why didn't we have one of these before?? */
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
					/* Distinguish between move numbers and things like R2xf4 */
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
								breakout2 = 1;
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
				case ' ': /* Finish reading the move. Break out of the loop */
					breakout = 1;
					break;
			}
		}

		x->next = y;
		x = y;
		ply++;
	}

	if (feof(finput)) {
		printf("*** Error: Move number %d by %s does not exist\n", move, (side == 'w')?"white":"black");
		exit(EXIT_FAILURE);
	}

	/* Now we have the list of moves loaded into "list" */

	/* We need to set up a board to record the game as it progress */
	/* This is the structure we'll use to display the first field of the FEN output */
	char board[RANKS][FILES] = {
		/* 8 */ {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},

		/* 7 */ {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},

		/* 6 */ {'1', '1', '1', '1', '1', '1', '1', '1'},

		/* 5 */ {'1', '1', '1', '1', '1', '1', '1', '1'},

		/* 4 */ {'1', '1', '1', '1', '1', '1', '1', '1'},

		/* 3 */ {'1', '1', '1', '1', '1', '1', '1', '1'},

		/* 2 */ {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},

		/* 1 */ {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}

		/*        a    b    c    d    e    f    g    h        */	
	};

	char castling = 17; /* Third field of the FEN: KQkq, each letter represents a bit, thus 17 is all castling allowed */
	int turn = WHITE;
	int enpassant = 0;

	x = list->next; /* Skip the empty item */
	while (x) {
	//	for (i = 0; x->move[i] != '\0'; i++)
//								printf("\"%s\"\n", x->move);	
		switch (x->move[0]) {
			case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h': /* Pawn move */
				if (strlen(x->move) > 2) { /* Move with capture */
					/* Set origin square */
					if (turn)
						board[RANKS - (x->move[3] - '0') + 1][x->move[0] - 'a'] = '1';
					else
						board[RANKS - (x->move[3] - '0') - 1][x->move[0] - 'a'] = '1';
					if (enpassant) { /* Clear the passed pawn */
						if (turn)
							board[RANKS - (x->move[3] - '0') + 1][x->move[2] - 'a'] = '1';
						else
							board[RANKS - (x->move[3] - '0') - 1][x->move[2] - 'a'] = '1';
						enpassant = 0;
					}
					/* Set destination square */
					/* Parenthesis are important, otherwise because RANKS is an int, the chars will get promoted and we'll get a wrong result */
					board[RANKS - (x->move[3] - '0')][x->move[2] - 'a'] = (turn)?'P':'p';
				} else {
					enpassant = 0;
					/* Set origin square */
					if (turn && '4' == x->move[1] && board[6][x->move[0] - 'a'] == 'P') { /* The pawn could've came from white's first move */
						board[6][x->move[0] - 'a'] = '1';
						enpassant = 1;
					}	else if (!turn && '5' == x->move[1] && board[1][x->move[0] - 'a'] == 'p') { /* The pawn could've came from black's first move */
						board[1][x->move[0] - 'a'] = '1';
						enpassant = 1;
					} else if (turn) /* White pawn push */
						board[RANKS - (x->move[1] - '0') + 1][x->move[0] - 'a'] = '1';
					else /* Black pawn push */
						board[RANKS - (x->move[1] - '0') - 1][x->move[0] - 'a'] = '1';
					/* Set destination square */
					board[RANKS - (x->move[1] - '0')][x->move[0] - 'a'] = (turn)?'P':'p';
				}
				break;
			case 'R':	case 'N':	case 'B': case 'Q': case 'K':
			case 'O':
				break;
		}
		x = x->next;
		turn = (turn)?BLACK:WHITE; /* Toggle turn */
	}

	/* Print the first field of the FEN */
	for (i = 0; i < RANKS-1; i++) {
		for (j = 0; j < FILES; j++)
			fprintf(foutput, "%c", board[i][j]);
		fprintf(foutput, "/");
	}
	/* The last rank doesn't have "/", so print it now */
	for (i = RANKS-1, j = 0; j < FILES; j++)
		fprintf(foutput, "%c", board[i][j]);


	exit(EXIT_SUCCESS);

}

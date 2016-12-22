/*
 *  pgn2fen - Extracts FEN of a specific move on a PGN game
 *  -------------------------------------------------------
 *  Author: mate_amargo - Juan Alberto Regalado Galván
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

#define NARGS 2						/* Mandatory arguments */
#define NARGSOPT 2				/* Optional arguments */
#define FILES 8						/* Board files (columns) (a-h) */
#define RANKS 8						/* Board ranks (rows) (1-8) */
#define CASTLEK (1 << 3)	/* White can castle Kingside */
#define CASTLEQ (1 << 2)	/* White can castle Queenside */
#define CASTLEk (1 << 1)	/* Black can castle Kingside */
#define CASTLEq 1 				/* Black can castle Queenside */
#define WHITE 1						/* Used to determine which turn... */
#define BLACK 0						/* ...is whilst traversing the list of moves */

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
		printf("\nNote: Since there's no way to get the initial position, i.e. before any player moves,\n");
		printf("I'll provide it in case you need that:\n");
		printf("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n");
		exit(EXIT_FAILURE);
	}
	/* Everything is ok, now let's work: */
	
	if (foutput == NULL) /* They didn't specify an output file so write to stdout */
		foutput = stdout;
	
	/* List to hold the moves */
	struct tlist {
		char move[8]; /* Max length if we don't count checks/mates is 6, e.g. exd8=Q, Nd7xe5 */
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
							case 'x': case 'O': case '-':	case '=':
								y->move[i++] = d;
								y->move[i++] = c;
								breakout2 = 1;
								break;
							case ' ':	case '\n': /* It's the ending of a move with number like e4 */
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
				case 'x': case 'O': case '-':	case '=':
					y->move[i++] = c;
					break;
				case ' ': case '\n': /* Finish reading the move. Break out of the loop */
					if (feof(finput)) /* We reached EOF while saving the last move! */
						clearerr(finput); /* Clear the EOF flag */
					if (strlen(y->move) > 0) /* Don't save empty lines */
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

	char castling = CASTLEK | CASTLEQ | CASTLEk | CASTLEq; /* Third field of the FEN: KQkq, each letter represents a bit */
	int turn = WHITE;
	int enpassant = 0;
	char target; /* Enpassant traget square. If it's a white pawn push the rank will always be 3, and 6 for black */
	char rook[2]; /* Rook origin for castling tests */
	int found = 0;
	ply = 0; /* Re-using variables, yay! This time it will be the ply clock, the fifth field of the FEN */

	x = list->next; /* Skip the empty item */
	while (x) {
		ply++; /* Advance halfmove clock. It could be reset later */
		switch (x->move[0]) {
			case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h': /* Pawn move */
				if (strstr(x->move,"=")) { /* Pawn promotion */
					for (i = 0; x->move[i] != '='; i++); /* i is now the position of "=" */
					if (turn) { /* White */
						/* Set origin square */
						board[1][x->move[0] - 'a'] = '1';
						/* Set destination square */
						board[0][x->move[i-2] - 'a'] = x->move[i+1];
						/* If the promotion occurs with a rook capture on the corner, the other color lose castling on that side */
						if ('a' == x->move[i-2])
							castling &= ~CASTLEq; /* Black Queenside */
						else if ('h' == x->move[i-2])
							castling &= ~CASTLEk; /* Black Kingside */
					} else { /* Black */
						/* Set origin square */
						board[RANKS-2][x->move[0] - 'a'] = '1';
						/* Set destination square */
						board[RANKS-1][x->move[i-2] - 'a'] = tolower(x->move[i+1]); /* Promotions are uppercase */
						if ('a' == x->move[i-2]) /* Check for rook capture */
							castling &= ~CASTLEQ; /* White Queenside */
						else if ('h' == x->move[i-2])
							castling &= ~CASTLEK; /* White Kingside */
					}
					enpassant = 0;
				} else if (strlen(x->move) > 2) { /* Move with capture */
					/* Set origin square */
					if (turn)
						board[RANKS - (x->move[3] - '0') + 1][x->move[0] - 'a'] = '1';
					else
						board[RANKS - (x->move[3] - '0') - 1][x->move[0] - 'a'] = '1';
					if (enpassant) { /* Clear the passed pawn */
						/* We need the origin, so we add (black) or subtract (white) 1 from the destination */
						/* In this case we don't need to translate the position to the matrix rank */
						if (turn && (x->move[3] - '0') - 1 == 5) /* If the capturing pawn is white it must be on rank 5 */
							board[RANKS - (x->move[3] - '0') + 1][x->move[2] - 'a'] = '1';
						else if ((x->move[3] - '0') + 1 == 4) /* If the capturing pawn is black it must be on rank 4 */
							board[RANKS - (x->move[3] - '0') - 1][x->move[2] - 'a'] = '1';
					}
					enpassant = 0;
					/* Set destination square */
					/* Parenthesis are important, otherwise because RANKS is an int, the chars will get promoted and we'll get a wrong result */
					board[RANKS - (x->move[3] - '0')][x->move[2] - 'a'] = (turn)?'P':'p';
				} else {
					enpassant = 0;
					/* Set origin square */
					if (turn && '4' == x->move[1] && board[6][x->move[0] - 'a'] == 'P') { /* The pawn could've came from white's first move */
						board[6][x->move[0] - 'a'] = '1';
						enpassant = 1;
						target = x->move[0];
					}	else if (!turn && '5' == x->move[1] && board[1][x->move[0] - 'a'] == 'p') { /* The pawn could've came from black's first move */
						board[1][x->move[0] - 'a'] = '1';
						enpassant = 1;
						target = x->move[0];
					} else if (turn) /* White pawn push */
						board[RANKS - (x->move[1] - '0') + 1][x->move[0] - 'a'] = '1';
					else /* Black pawn push */
						board[RANKS - (x->move[1] - '0') - 1][x->move[0] - 'a'] = '1';
					/* Set destination square */
					board[RANKS - (x->move[1] - '0')][x->move[0] - 'a'] = (turn)?'P':'p';
				}
				ply = 0; /* Pawn move or capture resets the halfmove clock */
				break;
			case 'R': /* Rook move */
				c = (turn)?'R':'r'; /* Set piece. Altough it would be clearer to define a new var "piece", I prefer to be confusing and reuse vars */
				/* Remove "x" if any */
				for (i = 0; x->move[i] != '\0'; i++)
					if (x->move[i] == 'x') {
						ply = 0; /* Capture resets the halfmove clock */
						for (j = i; x->move[j] != '\0'; j++)
							x->move[j] = x->move[j+1];
						break;
					}
				if (4 == strlen(x->move)) { /* Disambiguate move */
					/* Set origin square */
					if (x->move[1] > '8') { /* It's a letter, i.e. a file, then simply the origin file is given to us */
						if (x->move[1] == x->move[2]) { /* Same file move */
							for (found = 0, i = RANKS - (x->move[3] - '0') + 1; i < RANKS; i++) /* Look down on the file */
								if (board[i][x->move[1] - 'a'] == c) {
									board[i][x->move[1] - 'a'] = '1';
									rook[0] = x->move[1]; 
									sprintf(&rook[1],"%d", RANKS - i); 
									found = 1;
									break;
								} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
									break;
							for (i = RANKS - (x->move[3] - '0') - 1; !found && i >= 0; i--) /* Look up on the file */
								if (board[i][x->move[1] - 'a'] == c) {
									board[i][x->move[1] - 'a'] = '1';
									rook[0] = x->move[1]; 
									sprintf(&rook[1],"%d", RANKS - i); 
									break;
								} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
									break;
						} else { /* Same rank move */
							board[RANKS - (x->move[3] - '0')][x->move[1] - 'a'] = '1';
							rook[0] = x->move[1];
							rook[1] = x->move[3];
						}
					}	else { /* It's a number, i.e. the origin rank is given to us */
						if (x->move[1] == x->move[3]) { /* Same rank move */
							for (found = 0, i = (x->move[2] - 'a') + 1; i < FILES; i++) /* Look to the right on the rank */
								if (board[RANKS - (x->move[1] - '0')][i] == c) {
									board[RANKS - (x->move[1] - '0')][i] = '1';
									rook[0] = 'a' + i; 
									rook[1] = x->move[1]; 
									found = 1;
									break;
								} else if (board[RANKS - (x->move[1] - '0')][i] != '1') /* We hit a piece */
									break;
							for (i = (x->move[2] - 'a') - 1; !found && i >= 0; i--) /* Look to the left on the rank */
								if (board[RANKS - (x->move[1] - '0')][i] == c) {
									board[RANKS - (x->move[1] - '0')][i] = '1';
									rook[0] = 'a' + i; 
									rook[1] = x->move[1]; 
									break;
								} else if (board[RANKS - (x->move[1] - '0')][i] != '1') /* We hit a piece */
									break;
						} else { /* Same file move */
							board[RANKS - (x->move[1] - '0')][x->move[2] - 'a'] = '1';
							rook[0] = x->move[2];
							rook[1] = x->move[1];
						}
					}
					/* To set the destination square cut the extra letter */
					x->move[1] = x->move[2]; x->move[2] = x->move[3]; x->move[3] = '\0';
				} else { /* Unambigous move */
					/* Set origin square */
					for (found = 0, i = RANKS - (x->move[2] - '0') + 1; i < RANKS; i++) /* Look down on the file */
						if (board[i][x->move[1] - 'a'] == c) {
							board[i][x->move[1] - 'a'] = '1';
							rook[0] = x->move[1]; 
							sprintf(&rook[1],"%d", RANKS - i); 
							found = 1;
							break;
						} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
							break;
					for (i = RANKS - (x->move[2] - '0') - 1; !found && i >= 0; i--) /* Look up on the file */
						if (board[i][x->move[1] - 'a'] == c) {
							board[i][x->move[1] - 'a'] = '1';
							rook[0] = x->move[1]; 
							sprintf(&rook[1],"%d", RANKS - i); 
							found = 1;
							break;
						} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
							break;
					for (i = (x->move[1] - 'a') + 1; !found && i < FILES; i++) /* Look to the right on the rank */
						if (board[RANKS - (x->move[2] - '0')][i] == c) {
							board[RANKS - (x->move[2] - '0')][i] = '1';
							rook[0] = 'a' + i; 
							rook[1] = x->move[2]; 
							found = 1;
							break;
						} else if (board[RANKS - (x->move[2] - '0')][i] != '1') /* We hit a piece */
							break;
					for (i = (x->move[1] - 'a') - 1; !found && i >= 0; i--) /* Look to the left on the rank */
						if (board[RANKS - (x->move[2] - '0')][i] == c) {
							board[RANKS - (x->move[2] - '0')][i] = '1';
							rook[0] = 'a' + i; 
							rook[1] = x->move[2]; 
							break;
						} else if (board[RANKS - (x->move[2] - '0')][i] != '1') /* We hit a piece */
							break;
				}
				/* Set destination square */
				board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = c;
				/* Moving a rook cancels castling on that side */
				if 			(strcmp(rook,"h1") == 0) /* White Kingside */
						castling &= ~CASTLEK;
				else if (strcmp(rook,"h8") == 0) /* Black Kingside */
						castling &= ~CASTLEk;
				else if (strcmp(rook,"a1") == 0) /* White Queenside */
						castling &= ~CASTLEQ;
				else if (strcmp(rook,"a8") == 0) /* Black Queenside */
						castling &= ~CASTLEq;
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
				break;
			case 'N': /* Knight move */
				c = (turn)?'N':'n';
				/* Remove "x" if any */
				for (i = 0; x->move[i] != '\0'; i++)
					if (x->move[i] == 'x') {
						ply = 0; /* Capture resets the halfmove clock */
						for (j = i; x->move[j] != '\0'; j++)
							x->move[j] = x->move[j+1];
						break;
					}
				if (strlen(x->move) == 3) {
					/* Set origin square */
					found = 0;
					if (x->move[2] > '1') { /* Below, level -1 */
						found = 1;
						if (x->move[1] > 'b' && board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') - 2] == c)
							board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') - 2] = '1';
						else if (x->move[1] < 'g' && board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') + 2] == c)
							board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') + 2] = '1';
						else if (x->move[2] > '2') { /* Below, level -2 */
							if (x->move[1] > 'a' && board[RANKS - (x->move[2] - '0') + 2][(x->move[1] - 'a') - 1] == c)
								board[RANKS - (x->move[2] - '0') + 2][(x->move[1] - 'a') - 1] = '1';
							else if (x->move[1] < 'h' && board[RANKS - (x->move[2] - '0') + 2][(x->move[1] - 'a') + 1] == c)
								board[RANKS - (x->move[2] - '0') + 2][(x->move[1] - 'a') + 1] = '1';
							else
								found = 0;
						}	else
							found = 0;
					} 
					if (!found && x->move[2] < '8') { /* Above, level +1 */
						if (x->move[1] > 'b' && board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') - 2] == c)
							board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') - 2] = '1';
						else if (x->move[1] < 'g' && board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') + 2] == c)
							board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') + 2] = '1';
						else if (x->move[2] < '7') { /* Above, level +2 */
							if (x->move[1] > 'a' && board[RANKS - (x->move[2] - '0') - 2][(x->move[1] - 'a') - 1] == c)
								board[RANKS - (x->move[2] - '0') - 2][(x->move[1] - 'a') - 1] = '1';
							else if (x->move[1] < 'h' && board[RANKS - (x->move[2] - '0') - 2][(x->move[1] - 'a') + 1] == c)
								board[RANKS - (x->move[2] - '0') - 2][(x->move[1] - 'a') + 1] = '1';
						}
					}
					/* Set destination square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = c;
				} else if (strlen(x->move) == 4) {
					/* Set origin square */
					if (x->move[1] > '8') { /* It's a letter, i.e. a file, then simply the origin file is given to us */
						found = 0;
						if (abs(x->move[1] - x->move[2]) > 1) { /* "Horizontal move", like Nbd7, == 2 */
							if (x->move[3] < '8' && board[RANKS - (x->move[3] - '0') - 1][x->move[1] - 'a'] == c) {
								board[RANKS - (x->move[3] - '0') - 1][x->move[1] - 'a'] = '1';
								found = 1;
							}
							if (!found && x->move[3] > '1' && board[RANKS - (x->move[3] - '0') + 1][x->move[1] - 'a'] == c) {
								board[RANKS - (x->move[3] - '0') + 1][x->move[1] - 'a'] = '1';
								found = 1;
							}
						} else { /* "Vertical move", like Nfe6, == 1 */
							if (!found && x->move[3] < '7' && board[RANKS - (x->move[3] - '0') - 2][x->move[1] - 'a'] == c) {
								board[RANKS - (x->move[3] - '0') - 2][x->move[1] - 'a'] = '1';
								found = 1;
							}
							if (!found && x->move[3] > '2' && board[RANKS - (x->move[3] - '0') + 2][x->move[1] - 'a'] == c)
								board[RANKS - (x->move[3] - '0') + 2][x->move[1] - 'a'] = '1';
						}
					} else { /* It's a number, the rank is given */
						found = 0;
						if (abs(x->move[1] - x->move[3]) > 1) { /* "Vertical move", like N4e6, == 2 */
							if (x->move[2] > 'a' && board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - 1] == c) {
								board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - 1] = '1';
								found = 1;
							}
							if (!found && x->move[2] < 'h' && board[RANKS - (x->move[1] - '0')][(x->move[1] - 'a') + 1] == c) {
								board[RANKS - (x->move[1] - '0')][(x->move[1] - 'a') + 1] = '1';
								found = 1;
							}
						} else { /* "Horizontal move", like N4d5, == 1 */
							if (!found && x->move[2] > 'b' && board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - 2] == c) {
								board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - 2] = '1';
								found = 1;
							}
							if (!found && x->move[2] < 'g' && board[RANKS - (x->move[1] - '0')][(x->move[1] - 'a') + 2] == c)
								board[RANKS - (x->move[1] - '0')][(x->move[1] - 'a') + 2] = '1';
						}
					}
					/* Set destination square */
					board[RANKS - (x->move[3] - '0')][x->move[2] - 'a'] = c;
				} else { /* strlen(x->move) == 5), e.g. Nb4d5 */
					/* Set origin square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = '1';
					/* Set destination square */
					board[RANKS - (x->move[4] - '0')][x->move[3] - 'a'] = c;
				}
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
				break;
			case 'B': /* Bishop move */
				/* In the ridiculous case that they promote to bishop, we'll have to disambiguate.*/ 
				/* In the more ridiculous case that there's at least 3 same coloured bishops, we'll have rank and file, e.g. Bg8xd5 */
				c = (turn)?'B':'b';
				/* Remove "x" if any */
				for (i = 0; x->move[i] != '\0'; i++)
					if (x->move[i] == 'x') {
						ply = 0; /* Capture resets the halfmove clock */
						for (j = i; x->move[j] != '\0'; j++)
							x->move[j] = x->move[j+1];
						break;
					}
				if (strlen(x->move) == 3) {
					/* Set origin square */
					for (found = 0, i = 1; i <= RANKS - (x->move[2] - '0') && i <= (x->move[1] - 'a'); i++) /* Upper part  \ */
						if (board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] == c) {
							board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= RANKS - (x->move[2] - '0') && i <= FILES - (x->move[1] - 'a'); i++) /* Upper part / */
						if (board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] == c) {
							board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= (x->move[2] - '0') && i <= (x->move[1] - 'a'); i++) /* Lower part \ */
						if (board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] == c) {
							board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= (x->move[2] - '0') && i <= FILES - (x->move[1] - 'a'); i++) /* Lower part / */
						if (board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] == c) {
							board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] != '1') /* We hit a piece */
								break;
					/* Set destination square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = c;
				} else if (strlen(x->move) == 4) {
					/* Set origin square */
					if (x->move[1] > '8') { /* It's a letter, the origin file is given to us. Bbe4 */
						if (abs(x->move[1] - x->move[2]) <= RANKS - (x->move[3] - '0') && 
						board[RANKS - (x->move[3] - '0') - abs(x->move[1] - x->move[2])][x->move[1] - 'a'] == c) /* / */
							board[RANKS - (x->move[3] - '0') - abs(x->move[1] - x->move[2])][x->move[1] - 'a'] = '1';
						else if (abs(x->move[1] - x->move[2]) < (x->move[3] - '0') && 
						board[RANKS - (x->move[3] - '0') + abs(x->move[1] - x->move[2])][x->move[1] - 'a'] == c) /* \ */
							board[RANKS - (x->move[3] - '0') + abs(x->move[1] - x->move[2])][x->move[1] - 'a'] = '1';
					}	else { /* It's a number, i.e. the origin rank is given to us */
						if ((x->move[2] - 'a') + abs(x->move[3] - x->move[1]) < FILES && 
						board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') + abs(x->move[3] - x->move[1])] == c) /* / */
							board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') + abs(x->move[3] - x->move[1])] = '1';
						else if (abs(x->move[3] - x->move[1]) < (x->move[2] - 'a') && 
						board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - abs(x->move[3] - x->move[1])] == c) /* \ */
							board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - abs(x->move[3] - x->move[1])] = '1';
					}
					/* Set destination square */
					board[RANKS - (x->move[3] - '0')][x->move[2] - 'a'] = c;
				} else { /* strlen(x->move) == 5, e.g. Bf5g4*/
					/* Set origin square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = '1';
					/* Set destination square */
					board[RANKS - (x->move[4] - '0')][x->move[3] - 'a'] = c;
				}
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
				break;
			case 'Q': /* Queen move */
				c = (turn)?'Q':'q';
				/* Remove "x" if any */
				for (i = 0; x->move[i] != '\0'; i++)
					if (x->move[i] == 'x') {
						ply = 0; /* Capture resets the halfmove clock */
						for (j = i; x->move[j] != '\0'; j++)
							x->move[j] = x->move[j+1];
						break;
					}
				if (strlen(x->move) == 3) {
					/* Set origin square */
					/* Bishop-like: */
					for (found = 0, i = 1; i <= RANKS - (x->move[2] - '0') && i <= (x->move[1] - 'a'); i++) /* Upper part  \ */
						if (board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] == c) {
							board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') - i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= RANKS - (x->move[2] - '0') && i <= FILES - (x->move[1] - 'a'); i++) /* Upper part / */
						if (board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] == c) {
							board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') - i][(x->move[1] - 'a') + i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= (x->move[2] - '0') && i <= (x->move[1] - 'a'); i++) /* Lower part \ */
						if (board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] == c) {
							board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') - i] != '1') /* We hit a piece */
								break;
					for (i = 1; !found && i <= (x->move[2] - '0') && i <= FILES - (x->move[1] - 'a'); i++) /* Lower part / */
						if (board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] == c) {
							board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] = '1';
							found = 1;
							break;
						} else if	(board[RANKS - (x->move[2] -'0') + i][(x->move[1] - 'a') + i] != '1') /* We hit a piece */
								break;
					/* Rook-like: */
					for (i = RANKS - (x->move[2] - '0') + 1; !found && i < RANKS; i++) /* Look down on the file */
						if (board[i][x->move[1] - 'a'] == c) {
							board[i][x->move[1] - 'a'] = '1';
							found = 1;
							break;
						} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
							break;
					for (i = RANKS - (x->move[2] - '0') - 1; !found && i >= 0; i--) /* Look up on the file */
						if (board[i][x->move[1] - 'a'] == c) {
							board[i][x->move[1] - 'a'] = '1';
							found = 1;
							break;
						} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
							break;
					for (i = (x->move[1] - 'a') + 1; !found && i < FILES; i++) /* Look to the right on the rank */
						if (board[RANKS - (x->move[2] - '0')][i] == c) {
							board[RANKS - (x->move[2] - '0')][i] = '1';
							found = 1;
							break;
						} else if (board[RANKS - (x->move[2] - '0')][i] != '1') /* We hit a piece */
							break;
					for (i = (x->move[1] - 'a') - 1; !found && i >= 0; i--) /* Look to the left on the rank */
						if (board[RANKS - (x->move[2] - '0')][i] == c) {
							board[RANKS - (x->move[2] - '0')][i] = '1';
							break;
						} else if (board[RANKS - (x->move[2] - '0')][i] != '1') /* We hit a piece */
							break;
					/* Set destination square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = c;
				} else if (strlen(x->move) == 4) {
					/* Set origin square */
					/* Bishop-like: */
					if (x->move[1] > '8') { /* It's a letter, the origin file is given to us. Bbe4 */
						if (abs(x->move[1] - x->move[2]) <= RANKS - (x->move[3] - '0') && 
						board[RANKS - (x->move[3] - '0') - abs(x->move[1] - x->move[2])][x->move[1] - 'a'] == c) /* / */
							board[RANKS - (x->move[3] - '0') - abs(x->move[1] - x->move[2])][x->move[1] - 'a'] = '1';
						else if (abs(x->move[1] - x->move[2]) < (x->move[3] - '0') && 
						board[RANKS - (x->move[3] - '0') + abs(x->move[1] - x->move[2])][x->move[1] - 'a'] == c) /* \ */
							board[RANKS - (x->move[3] - '0') + abs(x->move[1] - x->move[2])][x->move[1] - 'a'] = '1';
					}	else { /* It's a number, i.e. the origin rank is given to us */
						if ((x->move[2] - 'a') + abs(x->move[3] - x->move[1]) < FILES && 
						board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') + abs(x->move[3] - x->move[1])] == c) /* / */
							board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') + abs(x->move[3] - x->move[1])] = '1';
						else if (abs(x->move[3] - x->move[1]) < (x->move[2] - 'a') && 
						board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - abs(x->move[3] - x->move[1])] == c) /* \ */
							board[RANKS - (x->move[1] - '0')][(x->move[2] - 'a') - abs(x->move[3] - x->move[1])] = '1';
					}
					/* Rook-like: */
					if (!found && x->move[1] > '8') { /* It's a letter, i.e. a file, then simply the origin file is given to us */
						if (x->move[1] == x->move[2]) { /* Same file move */
							for (i = RANKS - (x->move[3] - '0') + 1; i < RANKS; i++) /* Look down on the file */
								if (board[i][x->move[1] - 'a'] == c) {
									board[i][x->move[1] - 'a'] = '1';
									found = 1;
									break;
								} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
									break;
							for (i = RANKS - (x->move[3] - '0') - 1; !found && i >= 0; i--) /* Look up on the file */
								if (board[i][x->move[1] - 'a'] == c) {
									board[i][x->move[1] - 'a'] = '1';
									break;
								} else if (board[i][x->move[1] - 'a'] != '1') /* We hit a piece */
									break;
						} else /* Same rank move */
							board[RANKS - (x->move[3] - '0')][x->move[1] - 'a'] = '1';
					}	else if (!found) { /* It's a number, i.e. the origin rank is given to us */
						if (x->move[1] == x->move[3]) { /* Same rank move */
							for (i = (x->move[2] - 'a') + 1; i < FILES; i++) /* Look to the right on the rank */
								if (board[RANKS - (x->move[1] - '0')][i] == c) {
									board[RANKS - (x->move[1] - '0')][i] = '1';
									found = 1;
									break;
								} else if (board[RANKS - (x->move[1] - '0')][i] != '1') /* We hit a piece */
									break;
							for (i = (x->move[2] - 'a') - 1; !found && i >= 0; i--) /* Look to the left on the rank */
								if (board[RANKS - (x->move[1] - '0')][i] == c) {
									board[RANKS - (x->move[1] - '0')][i] = '1';
									break;
								} else if (board[RANKS - (x->move[1] - '0')][i] != '1') /* We hit a piece */
									break;
						} else /* Same file move */
							board[RANKS - (x->move[1] - '0')][x->move[2] - 'a'] = '1';
					}
					/* Set destination square */
					board[RANKS - (x->move[3] - '0')][x->move[2] - 'a'] = c;
				} else { /* strlen(x->move) == 5, e.g. Qf5g4*/
					/* Set origin square */
					board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = '1';
					/* Set destination square */
					board[RANKS - (x->move[4] - '0')][x->move[3] - 'a'] = c;
				}
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
				break;
			case 'K': /* King move */
				/* No need to disambiguate! There's never more than 1 king per-side. Halleluja! */
				c = (turn)?'K':'k';
				/* Remove "x" if any */
				for (i = 0; x->move[i] != '\0'; i++)
					if (x->move[i] == 'x') {
						ply = 0; /* Capture resets the halfmove clock */
						for (j = i; x->move[j] != '\0'; j++)
							x->move[j] = x->move[j+1];
						break;
					}
				/* Set origin square */
				found = 0;
				if (x->move[2] > '1') { /* Below */
					found = 1;
					if (board[RANKS - (x->move[2] - '0') + 1][x->move[1] - 'a'] == c)
						board[RANKS - (x->move[2] - '0') + 1][x->move[1] - 'a'] = '1';
					else if (x->move[1] > 'a' && board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') - 1] == c)
						board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') - 1] = '1';
					else if (x->move[1] < 'h' && board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') + 1] == c)
						board[RANKS - (x->move[2] - '0') + 1][(x->move[1] - 'a') + 1] = '1';
					else
						found = 0;
				}
				if (!found && x->move[2] < '8') { /* Above */
					found = 1;
					if (board[RANKS - (x->move[2] - '0') - 1][x->move[1] - 'a'] == c)
						board[RANKS - (x->move[2] - '0') - 1][x->move[1] - 'a'] = '1';
					else if (x->move[1] > 'a' && board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') - 1] == c)
						board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') - 1] = '1';
					else if (x->move[1] < 'h' && board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') + 1] == c)
						board[RANKS - (x->move[2] - '0') - 1][(x->move[1] - 'a') + 1] = '1';
					else
						found = 0;
				}
				if (!found && x->move[1] > 'a' && board[RANKS - (x->move[2] - '0')][(x->move[1] - 'a') - 1] == c) { /* Left */
					board[RANKS - (x->move[2] - '0')][(x->move[1] - 'a') - 1] = '1';
					found = 1;
				}
				if (!found && x->move[1] < 'h' && board[RANKS - (x->move[2] - '0')][(x->move[1] - 'a') + 1] == c) /* Right */
					board[RANKS - (x->move[2] - '0')][(x->move[1] - 'a') + 1] = '1';
				/* Set destination square */
				board[RANKS - (x->move[2] - '0')][x->move[1] - 'a'] = c;
				/* If you move your king you lose castling privileges */
				if (turn)
					castling &= ~(CASTLEK | CASTLEQ);
				else
					castling &= ~(CASTLEk | CASTLEq);
				break;
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
			case 'O': /* Castling */
				if (strlen(x->move) == 3) /* O-O */
					if (turn) { /* I could do smth crazy to save the if, like board[7*(1-turn)][]. But 1 "if" is faster than 4 multiplications, is it? */
						board[7][4] = '1';
						board[7][5] = 'R';
						board[7][6] = 'K';
						board[7][7] = '1';
					} else {
						board[0][4] = '1';
						board[0][5] = 'r';
						board[0][6] = 'k';
						board[0][7] = '1';
					}
				else /* strlen(x->move) == 5; O-O-O */ 
					if (turn) {
						board[7][0] = '1';
						board[7][2] = 'K';
						board[7][3] = 'R';
						board[7][4] = '1';
					} else {
						board[0][0] = '1';
						board[0][2] = 'k';
						board[0][3] = 'r';
						board[0][4] = '1';
					}
				/* If you have castled, then you can't castle anymore */
				if (turn)
					castling &= ~(CASTLEK | CASTLEQ);
				else
					castling &= ~(CASTLEk | CASTLEq);
				break;
				enpassant = 0; /* Piece move cancels enpassant opportunity*/
		}
		x = x->next;
		turn = (turn)?BLACK:WHITE; /* Toggle turn */
	}

	/* Print the first field of the FEN */
	for (i = 0; i < RANKS-1; i++) {
		c = '0'; /* We'll accumulate the 1's in "c". Reset it for every rank */
		for (j = 0; j < FILES; j++) 
			if ('1' == board[i][j])
				c++; /* ;-P */ 
			else { 
				fprintf(foutput, "%c%c", (c != '0')?c:'\0', board[i][j]); /* If we haven't accumulated 1's, don't print c */
				c = '0';
			}
		if (c > '0') /* We finished the loop with accumulated 1's! Print it */
				fprintf(foutput, "%c", c);
		fprintf(foutput, "/");
	}
	/* The last rank doesn't have "/", so print it now */
	c = '0';
	for (i = RANKS-1, j = 0; j < FILES; j++)
		if ('1' == board[i][j])
			c++; /* ;-P */ 
		else { 
			fprintf(foutput, "%c%c", (c != '0')?c:'\0', board[i][j]); /* If we haven't accumulated 1's, don't print c */
			c = '0';
		}
	if (c > '0') /* We finished the loop with accumulated 1's! Print it */
		fprintf(foutput, "%c", c);

	/* Print the second field of the FEN */
	/* This field uses the opposite interpretation that we used for the argument "side" */
	/* In ours, "w" means: position after white's move (meaning black moves next) */
	fprintf(foutput, " %c ", (side == 'w')?'b':'w');

	/* Print the third field of the FEN */
	if (!castling)
		fprintf(foutput, "- ");
	else	
		fprintf(foutput, "%c%c%c%c ", (castling & CASTLEK)?'K':'\0', (castling & CASTLEQ)?'Q':'\0', 
																	(castling & CASTLEk)?'k':'\0', (castling & CASTLEq)?'q':'\0');

	/* Print the fourth field of the FEN */
	if (enpassant)
		fprintf(foutput, "%c%d ", target, (side == 'w')?3:6);
	else
		fprintf(foutput, "- ");

	/* Print the fifth field of the FEN */
	fprintf(foutput, "%d ", ply);
	
	/* Print the sixth field of the FEN */
	/* Full move: It starts at 1, and is incremented after Black's move. We have the move number in "move" */
	/* All we need to do is add 1 to that if it's the position after black moved, as indicated by "side" */
	fprintf(foutput, "%d\n", (side == 'w')?move:move+1);

	exit(EXIT_SUCCESS);

}

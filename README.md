# pgn2fen
Extracts FEN of a specific move on a PGN game

Compile:
-------
There's a Makefile just for that purpose, simply run:
make

Instalation:
-----------
Sorry, no install commands or scripts. Just have fun, if you like it, install it by hand :)

Running the program:
--------------------
Running the program with no arguments, or with an invalid set of arguments will produce the following help:

Usage: ./pgn2fen input_game.pgn move [w/b] [output_position.fen]
  input_game.pgn       - A chess game in PGN format.
  move                 - A move number.
  w/b                  - OPTIONAL. Position reached after (w)hite or (b)lack move. Defaults to w.
  output_position.fen  - OPTIONAL. Output file. If not specified the output will be written to stdout.


For example, if game.png contains:
1. e4 c5 2. Nf3 d6
To print the position after white's second move:
./pgn2fen game.pgn 2
rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2
To print the position after black's second move:
./pgn2fen game.pgn 2 b
rnbqkbnr/pp2pppp/3p4/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3

Note: Since there's no way to get the initial position, i.e. before any player moves,
I'll provide it in case you need that:
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1


About PGN
=========

Portable Game Notation (PGN) is a plain text computer-processible format for recording chess games (both the moves and related data), supported by many chess programs.

PGN code begins with a set of "tag pairs" (a tag name and its value), followed by the "movetext" (chess moves with optional commentary). Moves are recorded using Standard Algebraic Notation.


About FEN
=========

Forsyth–Edwards Notation (FEN) is a standard notation for describing a particular board position of a chess game. The purpose of FEN is to provide all the necessary information to restart a game from a particular position.

A FEN "record" defines a particular game position, all in one text line and using only the ASCII character set.

A FEN record contains six fields. The separator between fields is a space. The fields are:

1. Piece placement (from white's perspective). Each rank is described, starting with rank 8 and ending with rank 1; within each rank, the contents of each square are described from file "a" through file "h". Following the Standard Algebraic Notation (SAN), each piece is identified by a single letter taken from the standard English names (pawn = "P", knight = "N", bishop = "B", rook = "R", queen = "Q" and king = "K"). White pieces are designated using upper-case letters ("PNBRQK") while black pieces use lowercase ("pnbrqk"). Empty squares are noted using digits 1 through 8 (the number of empty squares), and "/" separates ranks.

2. Active color. "w" means White moves next, "b" means Black.

3. Castling availability. If neither side can castle, this is "-". Otherwise, this has one or more letters: "K" (White can castle kingside), "Q" (White can castle queenside), "k" (Black can castle kingside), and/or "q" (Black can castle queenside).

4. En passant target square in algebraic notation. If there's no en passant target square, this is "-". If a pawn has just made a two-square move, this is the position "behind" the pawn. This is recorded regardless of whether there is a pawn in position to make an en passant capture.

5. Halfmove clock: This is the number of halfmoves since the last capture or pawn advance. This is used to determine if a draw can be claimed under the fifty-move rule.

6. Fullmove number: The number of the full move. It starts at 1, and is incremented after Black's move.

Examples:
---------

Here is the FEN for the starting position:

rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

Here is the FEN after the move 1. e4:

rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1

And then after 1. ... c5:

rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2

And then after 2. Nf3:

rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2

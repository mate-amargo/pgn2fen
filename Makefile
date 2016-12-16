CC := gcc

CFLAGS := -O2

pgn2fen : main.c
	$(CC) $(CFLAGS) main.c -o pgn2fen

.PHONY : clean

clean:
	$(RM) pgn2fen 

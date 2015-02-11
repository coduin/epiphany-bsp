CC = gcc
CFLAGS = -std=c99

all: hexviewer

bin/hexviewer: src/hexviewer.c
	$(CC) $(CFLAGS) -o $@ $< -lncurses

hexviewer: bin/hexviewer
	
########################################################

clean:
	rm bin/*

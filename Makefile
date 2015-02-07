CC=gcc

CFLAGS=-L. -L/usr/local/lib -lftd2xx


all: blink

blink: blink.c	
	$(CC) blink.c -o blink $(CFLAGS)

clean:
	rm -f *.o ; rm blink

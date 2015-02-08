include standard_rules.mk

all:
	make -C src
#blink: blink.o	
#	$(CC) blink.c -o blink $(CFLAGS)
	
#testloopback: testloopback.o
#	$(CC) testloopback.c -o testloopback $(CFLAGS)

clean:
	rm -f bin/* #Remove all files but Makefile

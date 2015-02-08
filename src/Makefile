include ../standard_rules.mk

OBJS= blink.o testloopback.o
BINS:= $(subst .o, ,$(OBJS))

all:  $(OBJS) $(BINS)

$(BINS):
	gcc $(CFLAGS) -o $(BIN_DIR)/$@ $@.o
	rm $@.o

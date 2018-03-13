CC=gcc
CFLAGS=-Wall -O0 -std=gnu99 -I/usr/local/include -g
LDFLAGS=-L/usr/local/lib -pthread -lm -lwiringPi

SOURCES=$(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SOURCES))
BINS = $(SOURCES:.c=)

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	$(RM) *.o *.a $(BINS)
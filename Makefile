CFLAGS = -D_GNU_SOURCE -Wall `pkg-config libusb-1.0 --cflags`
LDFLAGS = `pkg-config libusb-1.0 --libs`
CC = gcc
TARGETS = zinwell

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

zinwell: zinwell.o
	$(CC) $< $(LDFLAGS) -o $@

zinwell.o: zinwell.c
	$(CC) $< $(CFLAGS) -c

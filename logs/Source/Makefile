CFLAGS = -D_GNU_SOURCE -Wall `pkg-config libusb-1.0 --cflags`
LDFLAGS = `pkg-config libusb-1.0 --libs` -lpthread
CC = gcc
TARGETS = juca-isoctransfers_minimalcode

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

juca-isoctransfers_minimalcode: juca-isoctransfers_minimalcode.o
	$(CC) $< $(LDFLAGS) -o $@

juca-isoctransfers_minimalcode.o: juca-isoctransfers_minimalcode.c
	$(CC) $< $(CFLAGS) -c

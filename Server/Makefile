CC = gcc
CFLAGS = -Wall -pthread

TARGETS = queue roomsdh testsdh

all: $(TARGETS)

roomsdh: roomsdh.c
	$(CC) $(CFLAGS) roomsdh.c -o roomsdh

queue: queue.c
	$(CC) $(CFLAGS) queue.c -o queue

testsdh: testsdh.c
	$(CC) $(CFLAGS) testsdh.c -o testsdh
clean:
	rm -f $(TARGETS)
	rm *.txt

CC = gcc
CFLAGS = -Wall -pthread

TARGETS =  m_server s_client s_roomsdh m_client s_chk_performance s_queue s_testsdh

all: $(TARGETS)

m_server: m_server.c
	$(CC) $(CFLAGS) m_server.c -o m_server

s_client: s_client.c
	$(CC) $(CFLAGS) s_client.c -o s_client

s_roomsdh: s_roomsdh.c
	$(CC) $(CFLAGS) s_roomsdh.c -o s_roomsdh

m_client: m_client.c
	$(CC) $(CFLAGS) m_client.c -o m_client

s_chk_performance: s_chk_performance.c
	$(CC) $(CFLAGS) s_chk_performance.c -o s_chk_performance

s_queue: s_queue.c
	$(CC) $(CFLAGS) s_queue.c -o s_queue

s_testsdh: s_testsdh.c
	$(CC) $(CFLAGS) s_testsdh.c -o s_testsdh

clean:
	rm -f $(TARGETS)
	rm *.txt

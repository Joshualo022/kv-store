CC = gcc
CFLAGS = -Wall
LIBS = -lws2_32
TARGET = kv_server
SOURCES = src/main.c src/server.c src/hash_table.c src/protocol.c src/log.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET).exe
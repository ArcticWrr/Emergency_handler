# Makefile

CC = gcc
CFLAGS = -std=c11 -pthread 
SERVER_TARGET = serverfile
CLIENT_TARGET = clientfile

# Sorgenti
SERVER_SRCS = \
	src/server/server.c \
	src/utils/priority_queue.c \
	src/server/threads.c \
	src/parsing/parse_rescuer.c \
	src/parsing/parse_emergency_types.c \
	src/utils/digital_twins.c \
	src/parsing/parse_env.c \
	src/logging/logger.c \
	src/other/other_functions.c \
	src/utils/support.c

CLIENT_SRCS = \
	src/client/client.c \
	src/parsing/parse_env.c \
	src/logging/logger.c \
	src/other/other_functions.c

.PHONY: all server client clean server_run 

all: server client

# Compilazione server
server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRCS)

# Compilazione client
client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRCS)

# Esegui server
server_run: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Pulizia oggetti e eseguibili
clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) *.o *~
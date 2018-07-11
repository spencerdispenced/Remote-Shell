CC = gcc
CFLAGS = -Wall -Werror -pedantic 
DEBUG_FLAGS = -g
STD = -std=c11 -D_POSIX_C_SOURCE=200112L
LIBS = -lpthread


all:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(STD) $(LIBS) server.c -o server
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(STD) $(LIBS) client.c -o client


clean:
	rm  *client server 





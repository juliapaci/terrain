CC = gcc
CFLAGS = -Wall -Wextra -ggdb -lm -lraylib
TARGET = terrain

terrain: *
	$(CC) $(CFLAGS) *.c -o $(TARGET)

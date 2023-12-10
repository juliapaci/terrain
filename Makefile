CC = gcc
CFLAGS = -Wall -Wextra -ggdb -lm -lraylib
TARGET = terrain

terrain: *.c *.h
	$(CC) $(CFLAGS) *.c -o $(TARGET)

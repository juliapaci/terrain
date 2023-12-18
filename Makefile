CC = gcc
CFLAGS = -Wall -Wextra -ggdb -lm -lraylib
TARGET = terrain

terrain: src/*
	$(CC) $(CFLAGS) src/* -o $(TARGET)

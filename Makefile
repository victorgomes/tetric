CC = gcc
CFLAGS = -Wall -Wextra -std=c23 -g
LDFLAGS = -lncurses

TARGET = tetric

.PHONY: all clean

all: $(TARGET)

$(TARGET): tetric.c
	$(CC) $(CFLAGS) -o $(TARGET) tetric.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
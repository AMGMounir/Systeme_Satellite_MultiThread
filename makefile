CC = gcc
CFLAGS = -Iinclude -pthread -Wall
SRC = $(wildcard src/*.c)
TARGET = thermal_satellite

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
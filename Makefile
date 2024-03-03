CC = clang
CFLAGS = -Wall -Wextra -pedantic `sdl2-config --cflags`

TARGET = audioimager
SRC = audioimager.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) `sdl2-config --libs`

clean:
	rm -f $(TARGET)

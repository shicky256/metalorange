CC = gcc

TARGET = satconv.exe
CFLAGS = -g -std=gnu99 -Wall -DEZXML_NOMMAP -DWINDOWS

OBJECTS = satconv.o ezxml.o qdbmp.o map.o sprite.o tile.o

all: $(TARGET)

clean:
	rm *.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lws2_32

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
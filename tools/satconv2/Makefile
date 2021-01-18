CC = gcc

TARGET = satconv
CFLAGS = -g -std=gnu99 -Wall

OBJECTS = satconv.o qdbmp.o sprite.o

all: $(TARGET)

clean:
	rm *.o

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
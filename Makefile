CC = g++
CFLAGS = -O2 -g -Wall -std=c++20

SOURCES = dine.cpp main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = dine

:$(TARGET)

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

clean:
	rm -rvf $(OBJECTS)
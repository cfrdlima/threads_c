CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

SOURCES = main.c gerenciador_cyber.c cliente.c simulacao.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = cyber_cafe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c cyber_cafe.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
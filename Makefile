# Variables
CC = gcc
CFLAGS = -Wall -g -Wno-unused-function
TARGET = 6502
SOURCES = 6502.c memory_64k.c test.c
OBJECTS = $(SOURCES:.c=.o)

# Define target
all: $(TARGET)

# Build the exectuable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Define dependencies if any

# Clean target
clean: 
	-rm -f $(TARGET) $(OBJECTS)

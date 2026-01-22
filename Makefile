# Variables
CC = gcc
CFLAGS = -Wall -g -Wno-unused-function
TARGET = 6502
SOURCES = test.c
DEPS = 6502.c chip_mem.c
OBJECTS = $(SOURCES:.c=.o)

# Define target
all: $(TARGET)

# Build the exectuable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Compile .c files into .o files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Define dependencies if any

# Clean target
clean: 
	-rm -f -i $(TARGET) $(OBJECTS)

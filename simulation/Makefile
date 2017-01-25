# Which compiler to use
CC = gcc

# Compiler flags go here.
CFLAGS = -std=gnu11 -g -O2 -Wall -Wextra -pedantic -Wno-unused-value

# Linker flags go here.
LDFLAGS = -lcomedi -lm -lpthread

# list of sources
ELEVSRC = lib/timer.c elev.c io.c driver.c control.c elevatorcontrol.c main.c

# program executable file name.
TARGET = elevator

# top-level rule, to compile everything.
all: $(TARGET)

# Define all object files.
ELEVOBJ = $(ELEVSRC:.c=.o)

# rule to link the program
$(TARGET): $(ELEVOBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

# rule for cleaning re-compilable files.
clean:
	rm -f $(TARGET) $(ELEVOBJ)

rebuild:	clean all

.PHONY: all rebuild clean

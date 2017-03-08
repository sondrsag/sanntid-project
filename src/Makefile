# Which compiler to use
CC = gcc

# Compiler flags go here.
CFLAGS = -std=gnu11 -g -O2 -Wall -Wextra -pedantic

# Linker flags go here.
LDFLAGS = -lcomedi -lm -lpthread

# list of sources
#ELEVSRC = timer.c elev.c io.c driver.c elevatorcontrol.c dyad.c network.c elcom.c serialize_data.c work_distribution.c main.c
ELEVSRC = timer.c elev.c io.c driver.c elevatorcontrol.c work_distribution.c main.c

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

networktest: dyad.c network.c networktest.c
	gcc -Wall -O2 -pedantic -std=c11 $+ -lm -lpthread -D NUM_ELEVATORS=3 -o $@.exe

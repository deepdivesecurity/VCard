# Compiler flags
DLFLAGS = -shared
FP = -fPIC
CC = gcc
CFLAGS = -Wall -g -std=c11 -Iinclude

# Directories
SRC = ./src/

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
	LIB := ../parserlib.dylib
# SNU/Linux Dependant Variables
else ifeq ($(UNAME_S), Linux)
	LIB := ../parserlib.so
endif

all: VCardParser.o HelperFunctions.o LinkedListAPI.o
	$(CC) $(DLFLAGS) -o $(LIB) VCardParser.o HelperFunctions.o LinkedListAPI.o

VCardParser.o:
	$(CC) $(FP) $(CFLAGS) -c $(SRC)VCardParser.c -o VCardParser.o

HelperFunctions.o:
	$(CC) $(FP) $(CFLAGS) -c $(SRC)HelperFunctions.c -o HelperFunctions.o

LinkedListAPI.o:
	$(CC) $(FP) $(CFLAGS) -c $(SRC)LinkedListAPI.c -o LinkedListAPI.o

clean:
	rm *.o || rm *.a || rm *.so

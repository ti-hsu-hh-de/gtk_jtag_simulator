SHELL=/bin/bash
CFLAGS=-Wall -I ../include
CC=gcc

OBJ-FILES := main.o main_window.o jtag_tap.o

all: jtag-simulator

%.o: %.c jtag_tap.h
	$(CC) `pkg-config --cflags gtk+-3.0` -c -o $@ $< $(CFLAGS)

jtag-simulator: $(OBJ-FILES)
	$(CC) `pkg-config --cflags gtk+-3.0` -o jtag-simulator $(OBJ-FILES) `pkg-config --libs gtk+-3.0`

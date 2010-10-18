#!/usr/bin/make -f
SRC=gui.c
CC=gcc
CFLAGS=-Wall -g `pkg-config --cflags --libs gtk+-2.0` -I/usr/local/include
LFLAGS=-lavcodec -lavformat 
DESTDIR=/usr/local
gui: gui.c
	$(CC) $(CFLAGS) $(SRC) $(LFLAGS) -o gui

.phony : clean install
	 
clean:
	@rm gui
	@echo Cleaned
install: gui
	@install -m 0755 gui $(DESTDIR)/bin


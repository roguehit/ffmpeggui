SRC=gui.c
CC=gcc
CFLAGS= -g `pkg-config --cflags --libs gtk+-2.0` -I/usr/local/include -L/usr/local/lib
LFLAGS= -lavcodec -lavformat
gui: gui.c
	$(CC) $(CFLAGS) $(SRC) $(LFLAGS) -o gui

.phony : clean
	 
clean:
	@rm gui
	@echo Cleaned

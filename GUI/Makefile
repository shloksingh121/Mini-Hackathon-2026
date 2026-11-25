CC ?= cc
PKG_CONFIG ?= pkg-config
GTK := gtk+-3.0
CFLAGS ?= -O2 -std=c11 -Wall -Wextra

.PHONY: all clean run
all: student_gui

student_gui: student_gui.c updated_main.c
	$(CC) $(CFLAGS) $(shell $(PKG_CONFIG) --cflags $(GTK)) student_gui.c -o student_gui $(shell $(PKG_CONFIG) --libs $(GTK)) -lm

run: student_gui
	./student_gui

clean:
	rm -f student_gui

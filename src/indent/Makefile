
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0

indentrees: indent.c
	gcc -o indentrees indent.c

install: indentrees
	/bin/cp indentrees $(LEMHOME)/bin/indentrees

clean:
	/bin/rm -f Makefile
	/bin/rm -f indentrees


LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
BERKELEY_DB=/usr/local/BerkeleyDB.3.1/
PERLBIN=/usr/bin/perl

CFLAGS=-Wall
INC_DIRS=$(BERKELEY_DB)/include
LIB_DIRS=$(BERKELEY_DB)/lib
LIBS=-ldb

syn_db_access: syn_db_access.c 
		gcc $(CFLAGS) -I$(INC_DIRS) -L$(LIB_DIRS) syn_db_access.c -o syn_db_access $(LIBS)

install:
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" prep_syn_db.pl > $(LEMHOME)/bin/prep_syn_db.pl
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" syn_db_create | \
	  perl -pe "s|\@BERKELEY_DB\@|$(BERKELEY_DB)|g" > $(LEMHOME)/bin/syn_db_create
	chmod 755 $(LEMHOME)/bin/prep_syn_db.pl
	chmod 755 $(LEMHOME)/bin/syn_db_create

clean:
	/bin/rm -f Makefile
	/bin/rm -f syn_db_access


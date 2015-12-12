
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
PERLBIN=/usr/bin/perl

install:
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" synify.pl > $(LEMHOME)/bin/synify
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" check_syntax.pl | \
	  perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" > $(LEMHOME)/bin/check_syntax
	chmod 755 $(LEMHOME)/bin/synify
	chmod 755 $(LEMHOME)/bin/check_syntax

clean:
	/bin/rm -f Makefile
	@echo "cleaned dict"

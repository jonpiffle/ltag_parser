
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
PERLBIN=/usr/bin/perl

install:
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" runparser.pl | \
	  perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" > $(LEMHOME)/bin/runparser
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" parser_client.pl > $(LEMHOME)/bin/parser_client
	chmod 755 $(LEMHOME)/bin/runparser
	chmod 755 $(LEMHOME)/bin/parser_client

clean:
	/bin/rm -f Makefile
	@echo "cleaned user"

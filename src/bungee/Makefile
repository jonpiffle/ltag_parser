
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
WISHBIN=/usr/bin/wish
PERLBIN=/usr/bin/perl

install:
	perl -pe "s|\@WISHBIN\@|$(WISHBIN)|g" bungee.tk | \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" > $(LEMHOME)/bin/bungee
	perl -pe "s|\@WISHBIN\@|$(WISHBIN)|g" showtrees.tk > $(LEMHOME)/bin/showtrees
	perl -pe "s|\@WISHBIN\@|$(WISHBIN)|g" viewtree.tk > $(LEMHOME)/bin/viewtree
	perl -pe "s|\@WISHBIN\@|$(WISHBIN)|g" xviewtree.tk | \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" > $(LEMHOME)/bin/xviewtree
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" pickfamily.pl > $(LEMHOME)/bin/pickfamily.pl
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" xtag.show.sh > $(LEMHOME)/bin/xtag.show
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" xtag.show.fam.sh > $(LEMHOME)/bin/xtag.show.fam
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" xtag.show.word.pl > $(LEMHOME)/bin/xtag.show.word
	chmod 755 $(LEMHOME)/bin/bungee
	chmod 755 $(LEMHOME)/bin/showtrees
	chmod 755 $(LEMHOME)/bin/viewtree
	chmod 755 $(LEMHOME)/bin/xviewtree
	chmod 755 $(LEMHOME)/bin/pickfamily.pl
	chmod 755 $(LEMHOME)/bin/xtag.show
	chmod 755 $(LEMHOME)/bin/xtag.show.fam
	chmod 755 $(LEMHOME)/bin/xtag.show.word

clean:
	/bin/rm -f Makefile
	@echo "cleaned bungee"


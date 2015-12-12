
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
PERLBIN=/usr/bin/perl
TAGGER=/mnt/unagi/nldb/adwait/dist/tagger/bin.SunOS/tagger /mnt/unagi/nldb/adwait/dist/0-18.project

install:
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" tagger_filter.pl | \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" | \
	  perl -pe "s|\@TAGGER\@|$(TAGGER)|g" > $(LEMHOME)/bin/tagger_filter
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" simple_filter.pl > $(LEMHOME)/bin/filter
	perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" no_filter.pl > $(LEMHOME)/bin/no_filter
	chmod 755 $(LEMHOME)/bin/tagger_filter
	chmod 755 $(LEMHOME)/bin/filter
	chmod 755 $(LEMHOME)/bin/no_filter

clean:
	/bin/rm -f Makefile
	@echo "cleaned filter"

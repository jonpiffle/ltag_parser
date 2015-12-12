
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
PERLBIN=/usr/bin/perl
PYACC=/usr/bin/yacc
YFLAGS=
FIX=./fix
RM=/bin/rm -f

install: treefile.pl skel_trees.pl etrees.pl
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" create_tree_data.pl | \
           perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" > $(LEMHOME)/bin/create_tree_data
	perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" patch_grammar.pl | \
	   perl -pe "s|\@PERLBIN\@|$(PERLBIN)|g" > $(LEMHOME)/bin/patch_grammar
	chmod 755 $(LEMHOME)/bin/create_tree_data
	chmod 755 $(LEMHOME)/bin/patch_grammar

treefile.pl: treefile.y Tree.pm tree.ph
		$(PYACC) $(YFLAGS) treefile.y
		$(FIX) y.tab.pl > treefile.pl
		$(RM) y.tab.pl

skel_trees.pl: skel_trees.y Tree.pm tree.ph
		$(PYACC) $(YFLAGS) skel_trees.y
		$(FIX) y.tab.pl > skel_trees.pl
		$(RM) y.tab.pl

etrees.pl: etrees.y Tree.pm tree.ph
		$(PYACC) $(YFLAGS) etrees.y
		$(FIX) y.tab.pl > etrees.pl
		$(RM) y.tab.pl

clean:
	/bin/rm -f Makefile
	/bin/rm -f y.tab.pl
	/bin/rm -f treefile.pl etrees.pl skel_trees.pl debug_trees.pl




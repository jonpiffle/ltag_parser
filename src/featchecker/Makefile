
LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
COMP=/usr/bin/g++
# check that g++ is from egcs-1.1b or later 

# should not have to modify below this point

# uncomment if compiling under Linux with gcc-2.7.x, comment otherwise 
MODS = -O -I$(INCDIR)

# uncomment if compiling under Solaris with gcc-2.7.x, comment otherwise
# MODS = -O -I$(INCDIR) -D HACKLIST

LIBDIR  = .
BINDIR  = .
SRCDIR  = .
INCDIR  = .
DATADIR = .
SCRIPTDIR = .

CHKR_HEADERS = $(INCDIR)/nl-java.h $(INCDIR)/nl-list.h $(INCDIR)/nl-stringassoc.h \
               $(INCDIR)/nl-featurenode.h \
               $(INCDIR)/nl-feattable.h $(INCDIR)/nl-tagforest.h

CHKR_OBJECTS = $(LIBDIR)/nl-featurenode.o \
               $(LIBDIR)/nl-feattable.o $(LIBDIR)/nl-tagforest.o

all:     $(BINDIR)/checker

chkr-library: $(LIBDIR)/libchkrobjs.so

$(LIBDIR)/nl-featurenode.o: $(SRCDIR)/nl-featurenode.cpp $(CHKR_HEADERS)
	$(COMP) -c $(MODS) $(SRCDIR)/nl-featurenode.cpp -o $(LIBDIR)/nl-featurenode.o
#	$(COMP) -D CHECKLEAKS -c $(MODS) $(SRCDIR)/nl-featurenode.cpp -o $(LIBDIR)/nl-featurenode.o
$(LIBDIR)/nl-feattable.o: $(SRCDIR)/nl-feattable.cpp $(CHKR_HEADERS)
	$(COMP) -c $(MODS) $(SRCDIR)/nl-feattable.cpp -o $(LIBDIR)/nl-feattable.o
$(LIBDIR)/nl-tagforest.o: $(SRCDIR)/nl-tagforest.cpp $(CHKR_HEADERS)
	$(COMP) -c $(MODS) $(SRCDIR)/nl-tagforest.cpp -o $(LIBDIR)/nl-tagforest.o

$(LIBDIR)/libchkrobjs.a: $(CHKR_OBJECTS)
	ar r $@ $?

$(BINDIR)/checker: $(SRCDIR)/main-checker.cpp $(LIBDIR)/libchkrobjs.a $(CHKR_HEADERS)
	$(COMP) $(MODS) $(SRCDIR)/main-checker.cpp $(LIBDIR)/libchkrobjs.a -o $(BINDIR)/checker

install: english-install
	/bin/cp -f $(BINDIR)/checker $(LEMHOME)/bin/

english-install: 
	$(SCRIPTDIR)/create-featdb \
	    $(LEMHOME)/data/english/treenames.dat \
	    $(LEMHOME)/data/english/treefeats.dat \
	  > $(LEMHOME)/data/english/xtag.gram.featdb

clean:
	/bin/rm -f Makefile
	/bin/rm -f $(BINDIR)/checker
	/bin/rm -f $(LIBDIR)/*.o
	/bin/rm -f $(LIBDIR)/*.a


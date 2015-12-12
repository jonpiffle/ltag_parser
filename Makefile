
## Top level Makefile
##
## Copyright (c) 2000 Anoop Sarkar
##
## This source code is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License 
## as published by the Free Software Foundation; either version 2 of 
## the License, or (at your option) any later version. 
## 
## The file COPYING in the current directory has a copy of the GPL. 
##
## This code comes WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##

RCSID='$Id: Makefile,v 1.24 2001/08/22 21:31:29 anoop Exp $'
PACKAGE=lem
VERSION=0.14.0
PROC=`uname -m`
LEMHOME=$(PWD)
FTPDIR=/ftp/pub/xtag/lem
DATE=`date`

## Needed for installation of binaries
## Edit the following locations to reflect your system
WISH=/usr/bin/wish
PERL=/usr/bin/perl
# WISH=/usr/bin/wish
# PERL=/usr/local/bin/perl

JAVACOMP=$(/usr/libexec/java_home)/bin/javac
JAVABIN=$(/usr/libexec/java_home)/bin/java
# JAVACOMP=/usr/local/jdk1.3.1/bin/javac
# JAVABIN=/usr/local/jdk1.3.1/bin/java

# For Redhat Linux 7.x or 6.x JDK Version 1.2.2 is recommended
# jdk1.3.1 can be installed on Redhat 7.1 if you have 
# an updated libsafe (v2.0 or greater)

# JAVACOMP=/usr/local/jdk1.2.2/bin/javac
# JAVABIN=/usr/local/jdk1.2.2/bin/java

TAGGER=/mnt/unagi/nldb/adwait/dist/tagger/bin.SunOS/tagger \
	/mnt/unagi/nldb/adwait/dist/0-18.project

## If installing binaries stop here.
## The following values are only needed for source installation
## Edit the following locations to reflect your system
## Check that the value for CPPCOMP is g++ is from egcs-1.1b or later

BERKELEY_DB=/usr/local/BerkeleyDB.3.1/
TAR=/usr/bin/tar
CPPCOMP=/usr/bin/g++
CCOMP=/usr/bin/gcc
PERLYACC=/usr/bin/yacc

## Use the following values for a typical Linux installation.
## Note that the location for BERKELEY_DB is the default installation.
## If you run Redhat 6.0 or later, then you can set the value of
## BERKELEY_DB to /usr and avoid installing the BerkeleyDB package.

# BERKELEY_DB=/usr/local/BerkeleyDB
# TAR=/bin/tar
# CPPCOMP=g++
# CCOMP=gcc
# PERLYACC=/usr/local/bin/byacc

## Common utils
RM=/bin/rm -f
RMDIR=/bin/rm -rf

## Should not need any editing below this point

install: begin-mesg install-scripts-makefiles install-scripts install-xtag-grammar end-mesg
	@echo "install done."

all: begin-mesg makefiles compile-src install-scripts compile-syntax end-mesg
	@echo "all done."

begin-mesg:
	@echo "installing at location:" $(LEMHOME)

end-mesg:
	@echo ""
	@echo "install complete"

makefiles: compile-src-makefiles install-scripts-makefiles

compile-src-makefiles:
	cd ./src/parser; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@BERKELEY_DB\@|$(BERKELEY_DB)|g" | \
	  perl -pe "s|\@CCOMP\@|$(CCOMP)|g" > Makefile
	cd ./src/featchecker; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@CPPCOMP\@|$(CPPCOMP)|g" > Makefile
	cd ./src/indent; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in > Makefile
	@echo "done creating source makefiles"

compile-src:
	cd ./src/parser; \
	  make; make install
	cd ./src/featchecker; \
	  make; make install
	cd ./src/indent; \
	  make; make install
	@echo "done compiling source"

install-scripts-makefiles:
	cd ./src/bungee; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" | \
	  perl -pe "s|\@WISH\@|$(WISH)|g" > Makefile
	cd ./src/db; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" | \
	  perl -pe "s|\@BERKELEY_DB\@|$(BERKELEY_DB)|g" > Makefile
	cd ./src/derivations; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" > Makefile
	cd ./src/dict; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" > Makefile
	cd ./src/filter; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@TAGGER\@|$(TAGGER)|g" | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" > Makefile
	cd ./src/grammar; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" | \
	  perl -pe "s|\@PERLYACC\@|$(PERLYACC)|g" > Makefile
	cd ./src/user; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	  perl -pe "s|\@PERL\@|$(PERL)|g" > Makefile
	#cd ./src/browser; \
	#  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	#  perl -pe "s|\@JAVACOMP\@|$(JAVACOMP)|g" | \
	#  perl -pe "s|\@JAVABIN\@|$(JAVABIN)|g" > Makefile
	#cd ./src/server; \
	#  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in | \
	#  perl -pe "s|\@JAVACOMP\@|$(JAVACOMP)|g" | \
	#  perl -pe "s|\@JAVABIN\@|$(JAVABIN)|g" > Makefile
	@echo "done creating makefiles for installation"

install-scripts:
	cd ./src/bungee; \
	  make install
	cd ./src/db; \
	  make install
	cd ./src/derivations; \
	  make install
	cd ./src/dict; \
	  make install
	cd ./src/filter; \
	  make install
	#cd ./src/grammar; \
	#  make install
	cd ./src/user; \
	  make install
	#cd ./src/browser; \
	#  make install
	#cd ./src/server; \
	#  make install
	@echo "done installing files"

install-xtag-grammar:
	cd ./data/english; env "LEMHOME=$(LEMHOME)" \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" english.grammar.in > english.grammar

compile-syntax: install-xtag-grammar ./data/english/syntax.db

./data/english/syntax.db: ./data/english/syntax.flat
	@echo "creating syntax database. might take a while..."
	cd ./data/english; \
	  env "LEMHOME=$(LEMHOME)" $(LEMHOME)/bin/syn_db_create syntax.flat
	@echo "finished creating database"

all-treebank: begin-mesg install-treebank-scripts compile-treebank-syntax end-mesg
	@echo "install done."

install-treebank: begin-mesg install-treebank-scripts install-treebank-grammar end-mesg
	@echo "install done."

install-treebank-scripts:
	cd ./src/treebank; \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" Makefile.in > Makefile; \
	  make install

install-treebank-grammar:
	cd ./data/treebank; env "LEMHOME=$(LEMHOME)" \
	  perl -pe "s|\@LEMHOME\@|$(LEMHOME)|g" treebank.grammar.in > treebank.grammar

compile-treebank-syntax: install-treebank-grammar
	@echo "creating syntax database. might take a while..."
	cd ./data/treebank; \
	  env "LEMHOME=$(LEMHOME)" $(LEMHOME)/bin/syn_db_create syntax.flat
	@echo "finished creating database"

distrib: clean exclude-standard-files create-tarball

distrib-treebank: clean exclude-standard-files create-tarball

exclude-standard-files:
	find ./bin/ -print > /tmp/exclude_files
	find . -name "*.bin" -print >> /tmp/exclude_files
	find . -name "*.db" -print >> /tmp/exclude_files
	find . -name "*.class" -print >> /tmp/exclude_files
	find . -name "*.o" -print >> /tmp/exclude_files
	find . -name "*.a" -print >> /tmp/exclude_files
	find . -name "*.so" -print >> /tmp/exclude_files
	find . -name "RCS" -print >> /tmp/exclude_files
	find . -name "CVS" -print >> /tmp/exclude_files

exclude-treebank-files:
	find ./data/treebank/* -print >> /tmp/exclude_files
	find ./src/treebank/* -print >> /tmp/exclude_files

create-tarball:
	perl -pe "s/\@PACKAGE\@/$(PACKAGE)/g" README.in | \
	  perl -pe "s/\@VERSION\@/$(VERSION)/g" | \
	  perl -pe "s/\@DATE\@/$(DATE)/g" > README
	$(TAR) -X /tmp/exclude_files -zcf /tmp/$(PACKAGE)-$(VERSION).tgz ./*
	mkdir $(PACKAGE)-$(VERSION)
	$(TAR) -C $(PACKAGE)-$(VERSION) -zxf /tmp/$(PACKAGE)-$(VERSION).tgz
	$(TAR) -zcf $(PACKAGE)-$(VERSION).tgz $(PACKAGE)-$(VERSION)/*
	$(RM) /tmp/$(PACKAGE)-$(VERSION).tgz
	$(RM) /tmp/exclude_files

bindistrib: 
	find ./src/parser/* -print > /tmp/exclude_files
	find ./src/featchecker/* -print >> /tmp/exclude_files
	find ./src/indent/* -print >> /tmp/exclude_files
	find . -name "INSTALL.src" -print >> /tmp/exclude_files
	$(TAR) -X /tmp/exclude_files -zcf /tmp/$(PACKAGE)-$(VERSION).$(PROC).tgz ./*
	mkdir $(PACKAGE)-$(VERSION).$(PROC)
	$(TAR) -C $(PACKAGE)-$(VERSION).$(PROC) -zxf /tmp/$(PACKAGE)-$(VERSION).$(PROC).tgz
	$(TAR) -zcf $(PACKAGE)-$(VERSION).$(PROC).tgz $(PACKAGE)-$(VERSION).$(PROC)/*
	$(RM) /tmp/$(PACKAGE)-$(VERSION).$(PROC).tgz
	$(RM) /tmp/exclude_files

clean: makefiles distclean dataclean
	$(RM) README
	$(RM) $(PACKAGE)-$(VERSION).tgz
	$(RMDIR) $(PACKAGE)-$(VERSION)

distclean:
	cd src/browser; make clean; $(RM) Makefile
	cd src/server; make clean; $(RM) Makefile
	cd src/bungee; make clean; $(RM) Makefile
	cd src/db; make clean; $(RM) Makefile
	cd src/derivations; make clean; $(RM) Makefile
	cd src/dict; make clean; $(RM) Makefile
	cd src/featchecker; make clean; $(RM) Makefile
	cd src/filter; make clean; $(RM) Makefile
	cd src/grammar; make clean; $(RM) Makefile
	cd src/indent; make clean; $(RM) Makefile
	cd src/parser; make clean; $(RM) Makefile
	cd src/user; make clean; $(RM) Makefile
	$(RM) ./bin/*

dataclean:
	find . -name "*~" -print -exec $(RM) {} \;
	find . -name "core" -print -exec $(RM) {} \;
	find . -name ".Zap*" -print -exec $(RM) {} \;
	$(RM) ./data/english/syntax.db
	$(RM) ./data/treebank/syntax.db

remote-distrib-sh:
	perl -pe "s/\@PACKAGE\@/$(PACKAGE)/g" $(LEMHOME)/utils/remote-distrib.in | \
	  perl -pe "s/\@VERSION\@/$(VERSION)/g" > $(LEMHOME)/utils/remote-distrib.sh
	chmod 755 $(LEMHOME)/utils/remote-distrib.sh

local-distrib: distrib remote-distrib-sh
	/bin/cp -f $(PACKAGE)-$(VERSION).tgz $(FTPDIR)
	cd $(PACKAGE)-$(VERSION); \
	  cp README $(FTPDIR)/README-$(VERSION); \
	  make all; \
	  make bindistrib; \
	  /bin/cp -f $(PACKAGE)-$(VERSION).$(PROC).tgz $(FTPDIR)
	make clean


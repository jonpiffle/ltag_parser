#!/bin/sh

PACKAGE='lem';
VERSION='0.14.0';

cd /home/anoop;
/bin/rm -f $PACKAGE-$VERSION.tgz;
/bin/rm -rf $PACKAGE-$VERSION;

ncftpget -d stdout ftp://ftp.cis.upenn.edu/pub/xtag/lem/$PACKAGE-$VERSION.tgz;

tar zxvf $PACKAGE-$VERSION.tgz;
cd $PACKAGE-$VERSION;

/bin/mv Makefile Makefile.orig;
./utils/localize_makefile.pl Makefile.orig > Makefile;
make all;
make bindistrib;

/bin/mv $PACKAGE-$VERSION.i686.tgz /home/anoop;






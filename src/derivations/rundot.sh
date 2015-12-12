#!/bin/sh
if (test $# = 0); then (
        echo "usage: rundot <derivation_file>"
); else (
	cat $1 | @LEMHOME@/bin/df2dot.pl | dot -Tps -o $1.ps
	gv $1.ps &
); fi

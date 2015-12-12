#!/bin/sh

if (test $# = 0); then (
   echo "usage: memdebug <filename>"
); else (
   echo $1
   echo "Number of allocs at linenumber: "
   grep $1 mem.allocs | awk '{print $4}' | sort | uniq -c | sort -nr
   echo "Number of frees at linenumber: "
   grep $1 mem.frees | awk '{print $3}' | sort | uniq -c | sort -nr
); fi


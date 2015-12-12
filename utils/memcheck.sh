#!/bin/sh

cat mem.allocs | awk '{print $1, $2}' | sort -n > /tmp/$$.1
cat mem.frees | awk '{print $1}' | sort -n > /tmp/$$.2
paste /tmp/$$.1 /tmp/$$.2 > /tmp/$$.3
cat /tmp/$$.3 | awk '{ s+= $2 } END { print "total memory = ", s/1024, "K" }'
echo -n "num alloc ="
cat /tmp/$$.3 | wc -l
echo -n "num frees ="
cat /tmp/$$.3 | awk '{ if ($1 == $3) { print $1, $2; } else { print "??", $2; }}' | wc -l
/bin/rm -f /tmp/$$.1
/bin/rm -f /tmp/$$.2
/bin/rm -f /tmp/$$.3


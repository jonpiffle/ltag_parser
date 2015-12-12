echo "Mem allocs: "
cat mem.allocs | gawk '{print $3}' | sort | uniq -c | sort -nr > /tmp/$$.memrun
cat /tmp/$$.memrun
cat /tmp/$$.memrun | gawk '{s+=$1} END {print "Total=", s}'
echo "Mem frees: "
cat mem.frees | gawk '{print $2}' | sort | uniq -c | sort -nr > /tmp/$$.memrun
cat /tmp/$$.memrun
cat /tmp/$$.memrun | gawk '{s+=$1} END {print "Total=", s}'
/bin/rm -f /tmp/$$.memrun

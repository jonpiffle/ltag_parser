#!/usr/bin/perl -w

my $linenum = 0;

my $index;
my $entry;
my $pos;
my $trees;
my $fs;

sub combine_entry_and_pos
{
  my ($entry, $pos) = @_;
  my $epos = '';
  my $esz = 0;
  my $psz = 0;

  @elist = split(' ', $entry);
  @plist = split(' ', $pos);

  $esz = $#elist + 1;
  $psz = $#plist + 1;

  if ($esz != $psz) {
    print STDERR "mismatch in number of entries and part-of-speech at line $linenum\n";
    print STDERR "skipping entry: $entry\n";
    return('');
  }

  for ($i = 0; $i < $esz; $i++)
  {
     $epos .= '<<ENTRY>>' . $elist[$i] . '<<POS>>' . $plist[$i];
  }

  return($epos);
}

sub print_entry
{
  my ($index, $entry, $pos, $trees, $fs) = @_;
  my $epos = combine_entry_and_pos($entry, $pos);

  if ($epos eq '') { return; }
  if ($index) {
     print "<<INDEX>>$index";
     print $epos;
     print $trees;
     print "<<FEATURES>>$fs" if ($fs);
  }
  print "\n";
}

$index = $entry = $pos = $trees = $fs = '';
while (<>)
{
  chomp;
  $linenum++;
  next if /^;/;
  next if /^\s*$/;
  if (/^INDEX:/) {
     print_entry($index, $entry, $pos, $trees, $fs) if ($index);
     $index = $entry = $pos = $trees = $fs = '';
     s/^INDEX:\s*//g;
     s/([^\/]*)\/.*/$1/;
     s/^\s*//g; s/\s*$//g;
     $index = $_;
  }
  if (/^ENTRY:/) {
     s/^ENTRY:\s*//g;
     s/^\s*//g; s/\s*$//g;
     $entry = $_;
  }
  if (/^POS:/) {
     s/^POS:\s*//g;
     s/^\s*//g; s/\s*$//g;
     $pos = $_;
  }
  if (/^FAM:/) {
     s/^FAM:\s*//g;
     s/^\s*//g; s/\s*$//g;
     $trees = '<<FAMILY>>' . $_;
  }
  if (/^TREES:/) {
     s/^TREES:\s*//g;
     s/^\s*//g; s/\s*$//g;
     $trees = '<<TREES>>' . $_;
  }
  if (/^FS:/) {
     s/^FS:\s*//g;
     s/^\s*//g; s/\s*$//g;
     $fs = $_;
  }
}
print_entry($index, $entry, $pos, $trees, $fs) if ($index);


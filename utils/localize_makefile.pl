#!/usr/bin/perl -w

%localize_tbl = 
    (
     'WISH'        => '/usr/bin/wish',
     'PERL'        => '/usr/bin/perl',
     'BERKELEY_DB' => '/usr/local/',
     'TAR'         => '/bin/tar',
     'CPPCOMP'     => 'g++',
     'CCOMP'       => 'gcc',
     'JAVACOMP'    => '/usr/local/jdk1.2.2/bin/javac',
     'JAVABIN'     => '/usr/local/jdk1.2.2/bin/java',
     'PERLYACC'    => '/usr/local/bin/byacc'
     );

while (<>) {
    chomp;
    foreach $key (keys %localize_tbl) {
	if (/^\Q$key\E/) {
	    s/^\Q$key\E=(.*)$/$key=$localize_tbl{$key}/;
	}
    }
    print;
    print "\n";
}

1;





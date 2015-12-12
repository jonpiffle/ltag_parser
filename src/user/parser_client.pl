#!@PERLBIN@

use IO::Socket;

$verbose = 1;
$usage = "usage: $0 <remote-host> <remote-port>\n";
$remote_host = shift or die $usage;
$remote_port = shift or die $usage;

$socket = IO::Socket::INET->new(PeerAddr => $remote_host,
				PeerPort => $remote_port,
				Proto    => "tcp",
				Type     => SOCK_STREAM)
    or die "could not connect to $remote_host:$remote_port : $@\n";

print STDERR "[Connected to $remote_host:$remote_port]\n" if ($verbose);

my $input = '';
while (defined ($line = <>)) {
    $input .= $line;
}

die "cannot fork: $!\n" unless defined ($kidpid = fork());

if ($kidpid) {

    while (defined ($line = <$socket>)) {
	print $line;
	if (eof($socket)) { last; }
    }
    print STDERR "[done]\n" if ($verbose);
    kill("TERM" => $kidpid);

} else {

    print $socket $input;

}

close($socket);

my $input;
my $group;
my $protocol;

while( $input = <STDIN> )
{
	chop $input;
	if( $input =~ /^\[.*\]$/ )
	{
		if( $input =~ /^\[(korn-(\d+)-\d+)\]$/ )
		{
			$group = $1;
		}
		else
		{
			$group = "";
		}
	}
	
	if( $input =~ /^protocol\=(.*)/ )
	{
		$protocol=$1;
		print "[$group]\n";
		if( $protocol eq "imaps" )
		{
			print "protocol=imap\n";
			print "ssl=true\n";
		}
		elsif( $protocol eq "pop3s" )
		{
			print "protocol=pop3\n";
			print "ssl=true\n";
		}
		else
		{
			print "ssl=false\n";
		}
	}
}

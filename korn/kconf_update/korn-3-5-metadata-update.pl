my $input;
my $group;
my $auth = "";
my $tls = "";
my $metadata = "";

while( $input = <STDIN> )
{
	chop $input;
	if( $input =~ /^\[.*\]$/ )
	{
		if( $metadata )
		{
			print "[$group]\n";
			print "metadata=$metadata\n";
			$metadata="";
		}
	
		if( $input =~ /^\[(korn-(\d+)-\d+)\]$/ )
		{
			$group = $1;
		}
		else
		{
			$group = "";
		}
		$auth="";
		$tls="";
	}
	
	if( $input =~ /^auth\=(.*)/ )
	{
		$metadata=$tls ? "auth=$1,tls=$tls" : "auth=$1";
		$auth=$1;
		print "# DELETE [$group]auth\n";
	}
	elsif( $input =~ /^tls\=(.*)/ )
	{
		$metadata=$auth ? "auth=$auth,tls=$1" : "tls=$1";
		$tls=$1;
		print "# DELETE [$tls]tls\n";
	}
}

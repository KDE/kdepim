#!/usr/bin/perl

# Conversion of mbox to Maildir.
# You must set $MAIL and $MAILDIR for this to work.
# There are no locks, and it removes the spool after. Lovely !
 
use Sys::Hostname;
$host = hostname;
 
$maildir  = "$ENV{MAILDIR}";
$mbox     = "$ENV{MAIL}";

die ("Please set the environment variable MAILDIR to point to a directory that
  I can access. Thanks.") unless (-e $maildir and chdir($maildir));

-d "cur" or mkdir("cur",0700) or die("Sorry, couldn't make dir 'cur'\n");
-d "new" or mkdir("new",0700) or die("Sorry, couldn't make dir 'new'\n");
-d "tmp" or mkdir("tmp",0700) or die("Sorry, couldn't make dir 'tmp'\n");

open(MBOX, "<$mbox") or die "Sorry, can't open your mail spool using the path given in the environment variable MAIL (\"$ENV{$MAIL}\")\n";

$t = time;

print STDERR "Copying messages\nFrom: $mbox\nTo:   $maildir\n";

while(<MBOX>) {

  if (/^From /) {
    $fn = sprintf("new/%d.$$\_%d.$host", $t, $seq++);
    open(NEWMAIL, ">$fn") or die("Sorry, couldn't write message");
    print STDERR ".";
    next;
  }

# Damn mbox format quotes lines beginning with 'From '.
  s/^>From /From /;

  print NEWMAIL or die("\nSorry, couldn't write message");
}

close(NEWMAIL);
close(MBOX);

unlink("$ENV{MAIL}");

print STDERR "\nCopied $seq messages OK\n";


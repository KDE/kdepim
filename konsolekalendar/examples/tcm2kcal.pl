#!/usr/bin/perl -w

# tcm2kcal.pl -- converts a Turner Classic Movies (TCM) calendar
#                web page into a KDE calendar file.
############################################################################
# Copyright (c) 2003-2004 Allen Winter <winter@kde.org>                    #
# All rights reserved.                                                     #
# This program is free software; you can redistribute it and/or modify     #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation; either version 2 of the License, or        #
# (at your option) any later version.                                      #
############################################################################
#
# Modify $konkal, $cal, and $TCM as necessary for your needs.
#
# Requirements:
#   lynx
#   konsolekalendar (from KDE).
#   Date-Calc perl module (from CPAN).
#
# Options: -d, delete entries from KDE calendar file
#          -c, change entries in KDE calendar by re-writing movie titles in all caps.
#          -m <month>, where month is an integer 1 thru 12
#                      default is current month
#          -y <year>, where year is an integer YYYY
#                     default is current year
#          -f <file>, where file is location of output KDE calendar
#                     default is $HOME/.kde/share/apps/korganizer/tcm.ics
#	   -w <konsolekalendar> where konsolekalendar is located

use strict;
use Env qw(HOME);
use Getopt::Std;
use Date::Calc qw(Today Month_to_Text
                  Add_Delta_DHMS Add_Delta_Days
                  Day_of_Week Day_of_Week_to_Text
                  Month_to_Text);

#location of konsolekalendar program
my($konkal)="/usr/bin/konsolekalendar";

#location of new TCM KDE calendar
my($cal)="$HOME/.kde/share/apps/korganizer/tcm.ics";

#default program execution mode
my($mode)="--add";

#current date information
my($year,$month,$d) = Today();

#parse command line
&parse();

my($mmonth)=Month_to_Text($month);

#location of TCM monthly calendar web page
my($TCM)="http://www.turnerclassicmovies.com/Schedule/Print/0,,$month-$year|0|,00.html";
print "Processing $TCM ...\n";

if( open(GET, "lynx -dump -nolist -connect_timeout=3 '$TCM'|") ) {
} else {
  die "Failed to get the TCM calendar page.\n";
}

if( $mode eq "--add" ) {
  if( ! -r $cal ) {
    if( system("$konkal --create --file $cal") ) {
      print "Failure Condition Encountered.  Unable to Create KDE Calendar.\n"; exit 1;
    }
  }
}

my($tday,$mday);
my($day)=1;
my($lastampm)="AM";
my($line)="";
my($event)="";
my($num)=0;
while($line=<GET>) {

  #prepare line
  chomp($line);
  $line=&rmleadwhite($line);

  #skip junk lines
  next if( ! $line );
  next if( $line =~ m/^\[/ );
  next if( $line =~ m/^All Times Eastern/ );
  next if( $line =~ m/^$mmonth[[:space:]]*$year/ );

  #start new day if line begins with a DD Sunday|Monday|...|Friday|Saturday
  ($tday,$mday) = split(" ",$line);
  if( $tday =~ m/^[0-9]*$/ ) {
    if( $tday >= 1 && $tday <= 31 ) {
      if( $mday =~ m/day$/ ) {
	&process() if( $event );
        print "New day starting: $tday $mday\n";
        $day = $tday;
	$event = "";
	next;
      }
    }
  }

  #start a new event if line begins with a HH:MM[[space]]AM|PM
  if( $line =~ m/^[0-9]*:[0-9][0-9][[:space:]]*[A,P]M/ ) {
    &process() if( $event );
    $event = "$line";
  } else {
    $event = $event . " " . $line;
  }
}

#process remaining event, if there is one.
&process() if( $event );

close(GET);
print "$num movies processed\n";

##### END MAIN #####

sub rmleadwhite() {
  #remove leading white space
  my($x)=@_;
  ($x)=~s+^[[:space:]]*++;
  return($x);
}

sub parse() {
  #parse command line

  our($opt_h, $opt_d, $opt_c, $opt_m, $opt_y, $opt_f, $opt_w);
  getopts('hdcm:y:f:w:');

  $mode="--delete" if( $opt_d );
  $mode="--change" if( $opt_c );
  $month=$opt_m if( $opt_m );
  $year=$opt_y if( $opt_y );
  $cal=$opt_f if( $opt_f );
  $konkal=$opt_w if( $opt_w );

#  if( $opt_h ) {
#   print "help here!\r\n"; 
#  } // if

}

sub process() {
  #remove any evil double quotes from the event string
  $event =~ s/\"//g;

  ### Compute starting date
  my($date) = sprintf("%4d-%02d-%02d",$year,$month,$day);

  ### Compute starting time
  my($ttime,$ampm) = split(" ",$event);
  my($hour,$min) = split(":",$ttime);

  # adjust the hour by am or pm
  $hour += 12 if( $ampm =~ m/[Pp][Mm]/ && $hour < 12 );
  $hour -= 12 if( $ampm =~ m/[Aa][Mm]/ && $hour == 12 );

  my($time) = sprintf("%02d:%02d",$hour,$min);

  # advance day (for the enddate) if we have moved from pm to am
  if($lastampm =~ m/[Pp][Mm]/ && $ampm =~ m/[Aa][Mm]/ ) {
    ($year, $month, $day) = Add_Delta_Days($year,$month, $day, 1);
  }

  # format start date and time for "greping" later
  my($gdate) = sprintf("\"%s %02d %s %4d\"",
                       Day_of_Week_to_Text(Day_of_Week($year,$month,$day)),
                       $day,
                       Month_to_Text($month),
                       $year);
  my($ghour) = $hour;
  if( $ghour == 12 ) {
    $ampm = "pm";
  }
  if( $ghour > 12 ) {
    $ghour -= 12;
    $ampm = "pm"
  }
  if( $ghour == 0 ) {
    $ghour = 12;
    $ampm = "am";
  }
  my($gtime) = sprintf("\"%02d:%02d %s\"",$ghour,$min,lc($ampm));

  ### Compute Movie End Datetime by adding Movie Duration to Start Datetime

  # derive duration
  my($duration) = $event;
  $duration =~ s/CC//g;
  $duration =~ s/LBX//g;
  $duration =~ s/DVS//g;
  my(@d) = reverse(split(" ",$duration));
  $duration=$d[0];
  $duration =~ s/m\.$//g;
  #print "DURATION COMPUTATION ERROR\n" if( $duration < 1 || $duration > 300);

  my($endyear,$endmonth,$endday,$endhh, $endmm, $endss) = Add_Delta_DHMS(
       $year,$month,$day,$hour,$min,0,
                    0,   0,    $duration, 0);
  # format end datetime
  my($enddate) = sprintf("%4d-%02d-%02d",$endyear,$endmonth,$endday);
  my($endtime) = sprintf("%02d:%02d",$endhh,$endmm);
  my($genddate) = sprintf("\"%s %02d %s %4d\"",
                          Day_of_Week_to_Text(Day_of_Week($endyear,$endmonth,$endday)),
                          $endday,
                          Month_to_Text($endmonth),
                          $endyear);
  $ampm = "am";
  if( $endhh == 12 ) {
    $ampm = "pm";
  }
  if( $endhh > 12 ) {
    $endhh -= 12;
    $ampm = "pm";
  }
  if( $endhh == 0 ) {
    $endhh = 12;
    $ampm = "am";
  }
  my($gendtime) = sprintf("\"%02d:%02d %s\"",$endhh,$endmm,lc($ampm));

  # Derive Movie Title
  my($tmp) = split("[)]",$event);
  my($tmp2,$title) = split("^[0-9]*:[0-9][0-9][[:space:]]*[A,P]M",$tmp);
  $title=&rmleadwhite($title);
  $title=$title . ")";
  if( $title =~ m/\([[:space:]]\)/ ) {
    print "SKIPPING MOVIE WITHOUT A TITLE\n";
    return;
  }

  my($gtitle) = "\"" . $title . "\"";

  # "Grep line".
  #  Due to events across multiple days, search for ending date/time only.
#  my($UID)=&find_uid("$gdate,$gtime,$genddate,$gendtime,$gtitle");
  my($UID)=&find_uid(",$genddate,$gendtime,$gtitle");

  # New Title for Change Mode
  $title=uc($title) if( $mode eq "--change" );

  print "EVENT start datetime=$date, $time, title=$title, end datetime=$enddate, $endtime, duration=$duration\n";
  $lastampm=$ampm;

  # Run konsolekalendar to insert/change/delete the event
  if( system("$konkal $mode $UID --file $cal --date $date --time $time --end-date $enddate --end-time $endtime --summary \"$title\" --description \"$event\"") ) {
    $mode =~ s/--//;
    print "Failure Condition Encountered.  Unable to $mode Event.\n"; exit 1;
  }

  $num++;
}

sub find_uid() {

  my($line);
  my($grepline) = shift;
  my($UID)="";

  if( $mode ne "--add" ) {
    if( open(VIEW, "$konkal --view --all --export-type csv --file $cal |") ) {
      while($line=<VIEW>) {
	if( index($line,$grepline) >= 0) {
	  my(@u) = reverse(split(",",$line));
	  chomp($u[0]);
	  $UID="--uid=$u[0]";
	  last;
	}
      }
      if( $UID eq "" ) {
	print "Failure Condition Encoutered.  Unable to locate Event $grepline in calendar.\n"; exit 1;
      }
    } else {
      die "Failed to view cal $cal.\n";
    }
    close(VIEW);
  }
  return($UID);
}

__END__

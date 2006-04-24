#!/usr/bin/perl -w

eval 'exec /usr/bin/perl -w -S $0 ${1+"$@"}'
    if 0; # not running under some shell
###############################################################################
# Converts holiday files in 'plan' format into our holiday RelaxNG schema.    #
# Copyright (C) 2006 by Allen Winter <winter@kde.org>                         #
#                                                                             #
# This program is free software; you can redistribute it and/or modify        #
# it under the terms of the GNU General Public License as published by        #
# the Free Software Foundation; either version 2 of the License, or           #
# (at your option) any later version.                                         #
#                                                                             #
# This program is distributed in the hope that it will be useful,             #
# but WITHOUT ANY WARRANTY; without even the implied warranty of              #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                #
# GNU General Public License for more details.                                #
#                                                                             #
# You should have received a copy of the GNU General Public License           #
# along with this program; if not, write to the Free Software                 #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. #
#                                                                             #
###############################################################################
#
# A program to help convert the holiday files in 'plan' format to our new
# RelaxNG schema for KDE4 and above.  Note that this program does _not_
# include a full-blown 'plan' format parser.  It is intended to be just
# smart enough to do the conversion once, and then forget about the
# 'plan' format forever.
#
# This program also creates the new holiday directory structure
# (see DESIGN document).
#
# Program options:
#   --help:          display help message and exit
#   --version:       display version information and exit
#   --quiet:         suppress all output messages\n";
#
use strict;
use Getopt::Long;
use File::Basename;

my($Prog) = 'plan2xml.pl';
my($Version) = '1.0';

my($help) = '';
my($version) = '';
my($quiet) = '';

#location of converted holiday files
my($outdir) = '/data/kde/trunk/KDE/kdepim/libkholidays_ng/holidays';
#basename (without suffix) of national holiday files
my($national) = 'National';

#country mapping
my(%cm) = (#list of current holiday files to be converted.
           #not all the world's countries are listed here
           "at" => "Austria",
           "au" => "Australia",
           "be" => "Belgium",
           "ca" => "Canada",
           "ch" => "Switzerland",
           "co" => "Columbia",
           "cz" => "Czech Republic",
           "de" => "Germany",
           "dk" => "Denmark",
           "ee" => "Estonia",
           "es" => "Spain",
           "fi" => "Finland",
           "fr" => "France",
           "gb" => "United Kingdom/England and Wales",
           "gt" => "Guatemala",
           "hu" => "Hungary",
           "ie" => "Ireland",
           "il" => "Israel",
           "is" => "Iceland",
           "it" => "Italy",
           "ja" => "Japan",  #why did we use "ja" instead of "jp"?
           "jp" => "Japan",
           "lt" => "Lithuania",
           "mx" => "Mexico",
           "nl" => "Netherlands",
           "no" => "Norway",
           "nz" => "New Zealand",
           "pl" => "Poland",
           "pt" => "Portugal",
           "py" => "Paraguay",
           "ro" => "Romania",
           "se" => "Sweden",
           "si" => "Slovenia",
           "sk" => "Slovakia",
           "th" => "Thailand",
           "us" => "United States",
           "uy" => "Uruguay",
           "cat" => "Spain/Catalonia",
           "catalan" => "Spain/Catalonia",
           "bavarian" => "Germany/Bavaria",
           "quebec" => "Canada/Quebec",
           "Suedtirol" => "Italy/South Tyrol"
          );

exit 1
if (!GetOptions('help' => \$help, 'version' => \$version, 'quiet' => \$quiet));

&Help() if ($help);
if ($#ARGV < 0){ &Help(); exit 0; }
&Version() if ($version);

my($f,$base,$newf);
my($tmp,$tld,$country,$region);
for $f (@ARGV) {
  $base = basename($f);
  next if ($base =~ m/BelgiumFrench/);
  next if ($base =~ m/frswiss/);

  ($tmp,$tld) = split("_",$base);
  $newf = $cm{$tld};  $newf =~ s/ /_/g;

  $region = "";
  ($country,$region) = split("/",$newf);
  &myMkdir("$outdir/$country");

  print "Converting... $base to $newf\n";
  if ($region) {
    $newf = "$outdir/$country/$region.xml";
  } else {
    $newf = "$outdir/$country/$national.xml";
  }
  &processFile($f,$newf);
}

sub processFile() {
  my($in,$out) = @_;
  my($line);
  my($ln)=0;
  print "Processing $in:\n";
  open(IN, "$in") || die "Couldn't open $in";
  while ($line = <IN>) {
    $ln++;
    chomp($line);
    next if ($line =~ m/^:/);
    next if ($line =~ m/^$/);
    next if ($line =~ m/^\s+$/);
    $line = &convert($line);
    if ($line =~ m/^ERROR:/) {
      print "$ln: $line\n";
      exit 1;
    } else {
      print "$line\n";
    }
  }
  close(IN);
}

sub convert() {
# skip christmas if it looks like a religious holiday, i.e., 25 Dec
# skip easter
# skip astro seasons
  my($line) = @_;
  my($name,$date,$weekend,$offset,$length);
  $name=$date=$weekend=$offset=$length="";

#  print " Line=$line\n";
  my(@field)=split(" ",$line);
  if (lc($field[0]) =~ m/small/) {
    #small doesn't seem to be used
    shift @field;
  }
  # Skip stringcolor field
  if (&checkColor($field[0]) || lc($field[0]) eq "weekend") {
    # colors aren't used either
    shift @field;
  }

  # Next field must start with a double-quote (").
  if ($field[0] !~ m/^"/ ) {
    return "ERROR: Name field must be enclosed in double-quotes";
  }

  my($i,$cnt) = 0;
  if ($field[0] !~ m/^""$/) {
    while($i <= $#field) {
      $name .= $field[$i];
      if ($field[$i] =~ m/"$/) {
        $i = $#field;
      } else {
        $name .= " ";
      }
      $i++;
      $cnt++;
    }
    splice(@field,0,$cnt);
  } else {
    shift @field;
  }
  $name =~ s/"//g;

  # Skip daynumbercolor field
  shift @field if (&checkColor($field[0]));

  #weekend
  $weekend="false";
  if (lc($field[0]) eq "weekend"){
    $weekend="true";
    shift(@field);
  }

  # "on"
  if (lc($field[0]) ne "on") {
    return "ERROR: Missing \"on\" field (found $field[0])";
  }
  shift(@field); #skip "on"
  $i = 1;
  $cnt = 0;
  $date = $field[0];
  while($i <= $#field) {
    if (lc($field[$i]) eq "plus" ||
        lc($field[$i]) eq "minus" ||
        lc($field[$i]) eq "length") {
      $i = $#field;
    } else {
      $date .= " ";
      $date .= $field[$i];
    }
    $i++;
    $cnt++;
  }
  splice(@field,0,$cnt);

  # might be done now.
  if ($#field <= 1) {
    return(&buildStr($name,$date,$weekend,$offset,$length));
  }

  if (lc($field[0]) eq "plus" || lc($field[0]) eq "minus") {
    $i = 1;
    $cnt = 0;
    my($offset) = $field[0];
    while($i <= $#field) {
      if (lc($field[$i]) eq "length") {
        $i = $#field;
      } else {
        $offset .= " ";
        $offset .= $field[$i];
      }
      $i++;
      $cnt++;
    }
    splice(@field,0,$cnt);

    # might be done now.
    if ($#field <= 1) {
      return(&buildStr($name,$date,$weekend,$offset,$length));
    }
  }

  if (lc($field[0]) eq "length") {
    $i = 1;
    $cnt = 0;
    my($length) = "";
    while($i <= $#field) {
      if (lc($field[$i]) eq "length") {
        $i = $#field;
      } else {
        $length .= $field[$i];
        $length .= " ";
      }
      $i++;
      $cnt++;
    }
    $length =~ s/\s$//;
    splice(@field,0,$cnt);

    # might be done now.
    if ($#field <= 1) {
      return(&buildStr($name,$date,$weekend,$offset,$length));
    }
  }

  return "ERROR: Strangeness detected";
}

sub myMkdir() {
  my($d) = @_;
  if (! -d "$d") {
    mkdir("$d") || die "$d: $!\n";
  }
}

sub checkColor() {
  my($color) = @_;

  if (lc($color) eq "black"  ||
      lc($color) eq "red"    ||
      lc($color) eq "green"  ||
      lc($color) eq "yellow" ||
      lc($color) eq "blue"   ||
      lc($color) eq "magenta"||
      lc($color) eq "cyan"   ||
      lc($color) eq "white") {
    return 1;
  }
  return 0;
}

sub monthName() {
  my($str) = @_;

  if (lc($str) =~ m/jan/ ||
      lc($str) =~ m/feb/ ||
      lc($str) =~ m/mar/ ||
      lc($str) =~ m/apr/ ||
      lc($str) =~ m/may/ ||
      lc($str) =~ m/jun/ ||
      lc($str) =~ m/jul/ ||
      lc($str) =~ m/aug/ ||
      lc($str) =~ m/sep/ ||
      lc($str) =~ m/oct/ ||
      lc($str) =~ m/nov/ ||
      lc($str) =~ m/dec/) {
    return 1;
  }
  return 0;
}

sub monthNumber() {
  my($str) = @_;

  if (lc($str) =~ m/jan/){return 1;}
  if (lc($str) =~ m/feb/){return 2;}
  if (lc($str) =~ m/mar/){return 3;}
  if (lc($str) =~ m/apr/){return 4;}
  if (lc($str) =~ m/may/){return 5;}
  if (lc($str) =~ m/jun/){return 6;}
  if (lc($str) =~ m/jul/){return 7;}
  if (lc($str) =~ m/aug/){return 8;}
  if (lc($str) =~ m/sep/){return 9;}
  if (lc($str) =~ m/oct/){return 10;}
  if (lc($str) =~ m/nov/){return 11;}
  if (lc($str) =~ m/dec/){return 12;}
  return 0;
}

sub dayName() {
  my($str) = @_;

  if (lc($str) =~ m/mon/ ||
      lc($str) =~ m/tue/ ||
      lc($str) =~ m/wed/ ||
      lc($str) =~ m/thu/ ||
      lc($str) =~ m/fri/ ||
      lc($str) =~ m/sat/ ||
      lc($str) =~ m/sun/) {
    return 1;
  }
  return 0;
}

sub dayNumber() {
  my($str) = @_;

  if (lc($str) =~ m/mon/){return 1;}
  if (lc($str) =~ m/tue/){return 2;}
  if (lc($str) =~ m/wed/){return 3;}
  if (lc($str) =~ m/thu/){return 4;}
  if (lc($str) =~ m/fri/){return 5;}
  if (lc($str) =~ m/sat/){return 6;}
  if (lc($str) =~ m/sun/){return 7;}
  return 0;
}

sub weekPos() {
  my($str) = @_;

  if (lc($str) =~ m/first/ ||
      lc($str) =~ m/second/ ||
      lc($str) =~ m/third/ ||
      lc($str) =~ m/fourth/ ||
      lc($str) =~ m/fifth/ ||
      lc($str) =~ m/last/) {
    return 1;
  }
  return 0;
}

sub beforeOrAfter() {
  my($str) = @_;

  if (lc($str) =~ m/before/ ||
      lc($str) =~ m/after/) {
    return 1;
  }
  return 0;
}

sub litDate() {
  my($str) = @_;

  if ($str =~ m+^[[:digit:]]{1,2}\.[[:digit:]]{1,2}+ || # month.day[.year]
      $str =~ m+^[[:digit:]]{1,2}/[[:digit:]]{1,2}+) { # month/day[/year]
    return 1;
  } else {
    return 0;
  }
}

sub buildStr() {
  my($n,$d,$w,$o,$l) = @_;
  $n=&convertName($n);
  $d=&convertDate($d);
  $l = &convertLength($l);
  $o = &convertOffset($o);
  my($h)="<holiday" . $l . ">";
  return "$h\n$n\n$d\nWEEKEND=[$w]\n$o\n</holiday>\n";
}

sub convertDate() {
  my($datestr) = @_;
  my($day,$month,$year);

  my($s) = "";
  if (lc($datestr) eq "easter") {
    return "  <easter datestr=\"$datestr\">";
  }
  if (&litDate($datestr)) {
    ($day,$month,$year) = split("\\.",$datestr);
    if ($day && $month) {
      $s = &fixedDate($day,$month,$year);
      $s = "  <fixdate datestr=\"$datestr\" $s>";
      return $s;
    } else {
      return "ERROR: Improper fixed date. datestr=\"$datestr\"";
    }
  }
  if (&litDate($datestr)) {
    ($month,$day,$year) = split("/",$datestr);
    if ($day && $month) {
      $s = &fixedDate($day,$month,$year);
      $s = "  <fixdate datestr=\"$datestr\" $s>";
      return $s;
    } else {
      return "ERROR: Improper fixed date. datestr=\"$datestr\"";
    }
  }
  if (&monthName($datestr) && !&weekPos($datestr) && !&dayName($datestr)) {
    ($month,$day,$year) = split(" ",$datestr);
    $year = &fixYear($year);
    if ($day && &monthName($day)) {
      my($t) = $month;
      $month = $day;
      $day = $t;
    }
    if ($day && $month) {
      $month = &monthNumber($month);
      $s = "  <fixdate datestr=\"$datestr\" day=\"$day\" month=\"$month\"";
      $s .= " year=\"$year\"" if ($year);
      $s .= ">";
      return $s;
    } else {
      return "ERROR: Improper fixed date. datestr=\"$datestr\"";
    }
  }
  if (&dayName($datestr) && &beforeOrAfter($datestr)) {
    my($bora,$d);
    ($day,$bora,$d) = split(" ",$datestr);
    if ($day && $d) {
      $d = &convertDate($d);
      $s = "  <posdate datestr=\"$datestr\" dayOfWeek=\"$day\" bora=\"$bora\" date=\"$d\"";
      $s .= ">";
      return $s;
    } else {
      return "ERROR: Improper before/after date format.  datestr=\"$datestr\"";
    }
  }
  return "DATESPEC \"$datestr\" UNPARSED";
}

sub fixedDate() {
  my($d,$m,$y) = @_;
  my($str);

  $y = &fixYear($y);
  $m =~ s/^0+//; #remove leading zeros from the month number
  $d =~ s/^0+//; #remove leading zeros from the day number
  $str = "day=\"$d\" month=\"$m\"";
  $str .= " year=\"$y\"" if ($y);
  return $str;
}

sub fixYear() {
  my($year) = @_;
  if ($year) {
    $year += 2000 if ($year < 100);
  }
  return $year;
}

sub convertName() {
  #TODO: language attribute
  my($name) = @_;
  if ($name) {
    return "  <name>$name</name>";
  } else {
    return "ERROR: Empty name string";
  }
}

sub convertLength() {
  my($str) = @_;
  #unit of length must be days
  my($num,$unit) = split(" ",$str);
  if ($num) {
    if ($unit =~ m/^[Dd][Aa][Yy]/) {
      return " length=$num";
    } else {
      return "ERROR: Bad length unit \"$unit\"";
    }
  } else {
    return "";
  }
}

sub convertOffset() {
  my($str) = @_;
  #unit of offset must be days
  my($direction,$num,$unit) = split(" ",$str);
  my($s);
  if ($direction) {
    if (lc($direction) eq "plus") {
      $s = "    <plus";
    } else {
      $s = "    <minus"
    }
    if ($num > 0) {
      if ($unit =~ m/^[Dd][Aa][Yy]/) {
        $s .= " days=$num>";
        return $s;
      } else {
        return "ERROR: Bad offset unit \"$unit\"";
      }
    } else {
      return "ERROR: Offset must be a positive integer";
    }
  } else {
    return "";
  }
}

#==============================================================================
# Help function: print help message and exit.
sub Help {
  &Version();
  print "Converts holiday files in 'plan' format into our holiday RelaxNG schema.\n\n";
  print "Usage: $Prog [OPTION]... FILE...\n";
  print "  --help             display help message and exit\n";
  print "  --version          display version information and exit\n";
  print "  --quiet            suppress all output messages\n";
  print "\n";
  exit 0 if $help;
}

# Version function: print the version number and exit.
sub Version {
  print "$Prog, version $Version\n";
  exit 0 if $version;
}

__END__

        The holiday format is: (optional parts are in [square brackets],
        nonterminals are in CAPS, alternatives are separated by |, everything
        must be entered in lower case)

           [small]  [STRINGCOLOR]  "name"  [DAYNUMBERCOLOR]
                                                  on  [DATE]  [OFFSET]  [LENGTH]

        (Although shown here on two lines, every holiday definition must be
        entered on a single line.)

        Available colors are black, red, green, yellow, blue, magenta, cyan,
        white, and weekend (the same color used for Saturday and Sunday).
        The string color is used for the name when printed into a day box; the
        day number color is used to alter the color of the day number (1..31)
        of the day box the holiday falls on. This can be used to promote a day
        to an official holiday by using the "weekend" color. If there is a day
        number color specified, but no string color, the string color is set to
        the day number color. The name can be empty, but the quotes must be
        present. There are several formats for DATE:

           DAY . MONTH [ . YEAR]
           MONTH / DAY [ / YEAR]
           DAY   MONTHNAME   [YEAR]
           MONTHNAME   DAY   [YEAR]
           [every   NTH]   WEEKDAY   [in   MONTH]
           WEEKDAY   before   LIT_DATE
           WEEKDAY   after   LIT_DATE
           easter
           pascha

        DAY, MONTH, YEAR, NTH, and NUMBER can be C expressions; in
        dates, they must be parenthesized.  The special values any and last are
        also available. Any valid DATE description specifying a single day may
        be converted to a NUMBER by enclosing it in square brackets [].
        MONTHNAME is january, february, etc;  WEEKDAY is monday, tuesday,
        etc.  NTH can alternatively be first, second, ..., fifth, last. The
        words on, every, day, and days are syntactic sugar without meaning.
        Easter is predefined because its definition is rather complicated.
        LIT_DATE stands for one of the first two alternatives, DAY.MONTH[.YEAR]
        or MONTH/DAY[/YEAR]. Pascha is the Christian Orthodox Easter.

        The OFFSET after DATE is "[plus | minus   NUMBER   days", and
        the LENGTH after that is "length   NUMBER   days". Offsets are
        useful for holidays relative to Easter, and lengths are useful for trade
        shows and vacations. Always define vacations last in the list so regular
        holidays override them.

        Dates can be converted to numbers by enclosing them in square brackets.
        For example, the number of days between Easter and May 1 can be
        computed with ([may 1] - [easter]). As with C expressions, bracketed
        expressions must be parenthesized.

        If you have /lib/cpp (see CPP_PATH in the Makefile), you can use #include
        statements to include additional external holiday files. The external files
        cannot be edited interactively with plan; use an editor.

        Examples:
           small  "Easter"  weekend  on  easter
           small  "Surprise"  blue  on  last  sunday  in  april  plus  1  day
           small  green  "xmas"  weekend  on  12/25
           ""  weekend  on  july  4
           magenta  "Payday"  on  any/last
           green  "Vacation"  on  20.6.93  length  28  days
           #include "/usr/local/lib/vacations"

        Restrictions: plus, minus, and length may not cross over to the next or
        previous year, you cannot define New Year's as "last/last plus 1 day".

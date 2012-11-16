#!/usr/bin/perl -w

#########################################################################################
# Updates the E35 NewsLog.txt file for a tag release.                                   #
# Copyright (c) 2010-2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com> #
# Author: Allen Winter <allen.winter@kdab.com>                                          #
#                                                                                       #
# This program is free software; you can redistribute it and/or modify                  #
# it under the terms of the GNU General Public License as published by                  #
# the Free Software Foundation; either version 2 of the License, or                     #
# (at your option) any later version.                                                   #
#                                                                                       #
# This program is distributed in the hope that it will be useful,                       #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                        #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                          #
# GNU General Public License for more details.                                          #
#                                                                                       #
# You should have received a copy of the GNU General Public License along               #
# with this program; if not, write to the Free Software Foundation, Inc.,               #
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                         #
#                                                                                       #
#########################################################################################

# Requires LWP::UserAgent::https
#  If you get error message: "501  Protocol scheme 'https' is not supported..."
#  then you need to install LPW::UserAgent::https

# Requires the Term::Prompt module, available as source from CPAN,
#    http://search.cpan.org/~allens/Term-Prompt-0.11/Prompt.pm
# also available as a package for the following:
#  Fedora => perl-Term-Prompt
#  Kubuntu, Debian? Arch?

#TODO:
# make sure the added issues have status=6 (in testing)
# automagically get a list of all the issues in testing the past few weeks
#  and preset the @Issues array to that list.
# a little help page (new option 8)
# perldoc


use strict;
use LWP::UserAgent;
use URI::Escape;
use POSIX qw (setlocale strftime LC_TIME);
use Env qw (EDITOR PAGER);
use Term::Prompt;

my($Prog) = 'update_newslog.pl';
my
 $VERSION = '0.96';    #split line so MakeMaker can find the version here

# do not allow Git work dirs
-l ".git/refs" && die "Seems you are running this from a Git work dir.\nPlease change to a real Git repo instead\n";

my($logFile) = "NewsLog.txt";
-f "$logFile" || die "Cannot locate the E35 NewsLog.txt.\nAre you in the E35 branch?\nIs your branch up-to-date?\n";

&Message("Welcome to $Prog v$VERSION");
&Message("Please wait a few seconds while we check the status of your current log...");

# ensure the log file has no local mods
!&logFileModified() || die "Cannot continue.\n$logFile has local modifications.\nPlease commit or revert your local modifications and try again.\n";

# ensure the log file is not outdated.
if (&logFileOutdated()) {
  &Warning("$logFile is out-of-date.");
  my($yorn) = &prompt("y", "Retrieve an updated copy? If you say no this program will exit.");
  if (!$yorn) {
    &Message("logfile fail] file is out-of-date\n");
    exit;
  }
  system("git pull -q");
}

&Message("[logfile ok]\n");

my $Ua = LWP::UserAgent->new;
my($bakFile) = "$logFile" . ".bak";
my($tmpFile) = "NewsLog-" . &stamp() . ".txt";

my(@Issues) = ();


# &initTestingList(); #currently broken

### User prompt loop
Prompt:
my $result = &prompt("m", {
                         prompt           => "Please make a selection",
                         title            => 'NewsLog Update Menu',
                         items            => [ qw (add-issue remove-issue view-added-issues save-no-commit commit-and-push start-over quit) ],
                         order            => 'down',
                         rows             => 7,
                         cols             => 1,
                         display_base     => 1,
			 return_base      => 1,
                         accept_multiple_selections => 0,
                         accept_empty_selection     => 0
                        },
		     "", #help string
		     ""  #default string
		    );


if ($result == 1) {
  &AddIssue();
} elsif ($result == 2) {
  &RemoveIssue();
} elsif ($result == 3) {
  &View();
} elsif ($result == 4) {
  &Save();
} elsif ($result == 5) {
  &Commit();
} elsif ($result == 6) {
  &StartOver();
} elsif ($result == 7) {
  &Quit();
}
goto Prompt;

##### Prompt Functions #####

sub AddIssue() {
 AddAnother:
  my($issue) = &promptIssue("add");
  return if($issue == 0);

  my($i);
  foreach $i (@Issues) {
    if ( $i == $issue ) {
      &Message("[add fail] kolab/issue$issue has been added already.\n");
      return;
    }
  }
  my($foo) = &issueString(0,$issue);
  if ("$foo" ne "") {
    push(@Issues,$issue);
    &Message("[add ok] kolab/issue$issue");
    &printIssueList();
  }
  goto AddAnother;
}

sub RemoveIssue() {
 RemoveAnother:
  if ($#Issues < 0) {
    &Message("No issues to remove.\n");
    return;
  }
  my($issue) = &promptIssue("remove");
  return if($issue == 0);

  my($i);
  my(@nIssues) = ();
  foreach $i (@Issues) {
    if ( $i != $issue ) {
      push(@nIssues,$i);
    }
  }
  if ($#Issues == $#nIssues+1) {
    @Issues = @nIssues;
    &Message("[remove ok] kolab/issue$issue");
    &printIssueList();
  } else {
    &Message("[remove fail] kolab/issue$issue has not been added yet.\n");
  }
  goto RemoveAnother if ($#Issues >= 0);
}

sub View() {
  if ($#Issues < 0) {
    &Message("No issues have been added. So there is nothing to view.\n");
    return;
  }
  my $viewer = "less";
  if ($PAGER) {
    $viewer = $PAGER;
  }
  &printTmpFile();
  system("$viewer $tmpFile");
}

sub Save() {
  &printTmpFile();
  system("git checkout -q -- $logFile");
  system("mv -f $logFile $bakFile");
  system("cat $tmpFile $bakFile >$logFile");
  &Message("[save ok]\n");
  return 1;
}

sub Commit() {
  if ($#Issues < 0) {
    &Message("[commit fail] No Issues have been added so there is nothing to commit.\n");
    return;
  }
  if ( &Save() ) {
    if(&prompt("y", "Would you like to edit the $logFile before committing?")) {
      &handEdit();
    }
    if(&prompt("y", "Last chance.. really commit and push?")) {
      system("git commit --message \"update for today's tagging\" -q $logFile");
      system("git push --all");
      &Message("[commit ok]\n");
      &Cleanup();
    }
  }
  return;
}

sub StartOver() {
  if ($#Issues >= 0) {
    my($i) = $#Issues + 1;
    my($yorn) = &prompt("y", "Are you sure you want to start over? There are $i issues in-progress?");
    if (!$yorn) {
      return;
    }
  }
  system("git -q checkout -- $logFile");
  &Cleanup();
}

sub Quit {
  if ($#Issues >= 0) {
    my($yorn) = &prompt("y", "Newly added issues haven't been committed yet. Quit anyway?");
    if (!$yorn) {
      return;
    }
  }
  &Cleanup();
  exit 0;
}

##### Utility Functions #####

# check to see if the logFile has local modifications.
sub logFileModified() {
  my($m) = `git status -s $logFile | grep '^ M'`;
  if (!defined($m) || !$m) {
    return 0;
  } else {
    return 1;
  }
}

# check to see if the logFile is out-of-date.
sub logFileOutdated() {
  my($m) = `git diff --name-only | grep $logFile`;
  if (!defined($m) || !$m) {
    return 0;
  } else {
    return 1;
  }
}

# cleanup stuff
sub Cleanup {
  unlink($tmpFile);
  @Issues = ();
}

# provide a normal user info message.
sub Message {
  my($message) = @_;
  print "\n$message\n";
  sleep 1;
}

# provide a user warning message
sub Warning {
  my($warning) = @_;
  print STDERR "\nWarning: $warning\n\n";
  sleep 1;
}

# allow for hand-editing the log file before committing
sub handEdit() {
  my $editor = "vi";
  if ($EDITOR) {
    $editor = $EDITOR;
  }
  system("$editor $logFile");
}

# prompt for an issue number.
# return 0 means user cancels; other return values are greater than 999.
sub promptIssue {
  my($type) = @_;
  my($issue) = 0;
  while(1) {
    if ($type eq "add") {
      $issue = &prompt("n", "Issue Number to Add?", "0 to escape add-issue", "");
    } else {
      $issue = &prompt("n", "Issue Number to Remove?", "0 to escape remove-issue", "");
    }
    if ($issue < 1000) {
      if ($issue > 0) {
	&Message("Sorry, issue numbers must be 1000 or greater.\n");
      } else {
	return 0;
      }
    } else {
      return $issue;
    }
  }
}

# print the output string to the temporary file
sub printTmpFile {
  unlink($tmpFile);
  open(F,">$tmpFile") || die "Failed to open $tmpFile";
  print F "\n";
  print F &asOf();
  print F "\n\n";
  print F "Problems addressed\n";
  print F "------------------\n";
  print F "\n";
  print F &outputStr();
  print F "\n\n";
  close(F);
}

# print the list of issues
sub printIssueList {
  print "[issues] " . &issueList() . "\n\n";
}

# create a short, displayable list of issues
sub issueList {
  my($str) = "";
  if ($#Issues >= 0) {
    for my $i (sort @Issues) {
      $str = $str . $i . ",";
    }
    $str =~ s/,$//;
  } else {
    $str = "(empty)";
  }
  return $str;
}

# create a string of all the issue strings, suitable for display and printing
sub outputStr {
  my($i);
  my($outStr) = "";
  for my $i (sort @Issues) {
    my($issueStr) = &issueString(1,$i);
    if($issueStr) {
      $outStr = $outStr . "* " . $issueStr . "\n";
    }
  }
  return $outStr;
}

# create an initial issues list from the set of those currently in testing
sub initTestingList {
  my($url) = 'https://roundup.kolab.org/issue?@columns=title,id,status&@action=export_csv&@sort=id&@pagesize=50&@startwith=0&status=6&assignedto=520,341&keyword=20';


#  $url = uri_escape($url);
  system("wget -O - --no-check-certificate $url");

#  my($req) = HTTP::Request->new(GET => $url);
#  $req->header(Accept => "If-SSL-Cert-Subject");

#  my($res) = $Ua ->request($req);

#  if ($res->is_success) {
#    my(@lines) = split('\n', $res->content);
#    for my $l (@lines) {
#      print "$l\n";
#    }
#  } else {
#    &Warning("cannot access list of issues in-testing.");
#  }
}


# create an issue string from the specified issue number
# if the force argument is non-zero, then don't check sanity
sub issueString {
  my($force,$issue) = @_;
  my($str) = "";

  my($url) = "https://roundup.kolab.org/issue" . $issue;

  my($req) = HTTP::Request->new(GET => $url);

  $Ua->ssl_opts( verify_hostname => 0 );
  my($res) = $Ua ->request($req);
  if ($res->is_success) {
    my($tmp,$tmp1);
    my($issue_pr,$summ_pr,$rt_pr);

    my(@lines) = split('\n', $res->content);
    my(@summary) = grep{/kolab\/issue$issue/} @lines;
    if ($#summary > 0) {
      ($issue_pr) = split(' ',$summary[0]);
      $rt_pr = "";
      if ($summary[0] =~ m/([Rr][Tt]\s*#\d{3,})/) {
         $rt_pr = "($1)";
      }
      $summ_pr = $summary[0];
      $summ_pr =~ s/\($rt_pr\)//;
      $summ_pr =~ s/kolab\/issue$issue\s*\(\s*//;
      $summ_pr =~ s/\s*[\)]+\s*$//;
      $summ_pr =~ s/&gt;/>/g;
      $summ_pr =~ s/&lt;/</g;
      if (!$rt_pr) {
	$str = "$issue_pr: $summ_pr";
	if (!$force) {
	  my($yorn) = &prompt("y", "Issue $issue does not have an RT#. Add it anyway?");
	  if (!$yorn) {
	    $str = "";
	  }
	}
      } else {
	$str = "$issue_pr: $summ_pr $rt_pr";
      }
    } else {
      print &Warning("issue $issue is empty.");
    }
  } else {
    print &Warning($res->status_line . ": cannot access issue $issue.");
  }

  return $str;
}

# return a timestamp string
sub stamp {
  my $locale = setlocale( LC_TIME );
  setlocale( LC_TIME, "C" );
  my $time = strftime( "%F", localtime( time() ) ); #ISO 8601 date format
  setlocale( LC_TIME, $locale );
  return $time;
}

# return a nicely formatted string containing the current time
sub asOf {
  my $locale = setlocale( LC_TIME );
  setlocale( LC_TIME, "C" );
  my $time = strftime( "%A %d %B %Y", localtime( time() ) );
  setlocale( LC_TIME, $locale );
  return $time;
}

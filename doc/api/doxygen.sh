#! /bin/sh

# A shell script that builds dox without all the tedious mucking about with
# autoconf and configure. Run it in the "top builddir" with one argument,
# the "top srcdir". Something like this:
#
# cd /mnt/build/kdepim
# sh /mnt/src/kdepim/doc/api/doxygen.sh /mnt/src/kdepim
#
# You can also build single subdirs (for instance, after updating some
# dox and you don't want to rebuild for the enitre module) by giving the
# subdirectory _relative to the top srcdir_ as a second argument:
#
# sh /mnt/src/kdepim/doc/api/doxygen.sh /mnt/src/kdepim kpilot/lib
# 
# When generating dox for kdelibs, a tag file for Qt is also created.
# The location of Qt is specified indirectly through $QTDOCDIR or,
# if that is not set, $QTDIR, or otherwise guessed.
#
# Finally, there is a --no-recurse option for top-level generation
# that avoids generating all the subdirectories as well. It also
# suppresses cleaning up (rm -rf) of the dox direction beforehand.


recurse=1

if test "x--no-recurse" = "x$1" ; then
	shift
	recurse=0
fi

if test -z "$1" ; then
	echo "Usage: doxygen.sh <top_srcdir>"
	exit 1
fi
if ! test -d "$1" ; then
	echo "top_srcdir ($1) is not a directory."
	exit 1
fi

if test -z "$QTDOCDIR" ; then
	if test -z "$QTDIR" ; then
		for i in /usr/X11R6/share/doc/qt/html
		do
			QTDOCDIR="$i"
			test -d "$QTDOCDIR" && break
		done
	else
		for i in share/doc/qt/html doc/html
		do
			QTDOCDIR="$QTDIR/$i"
			test -d "$QTDOCDIR" && break
		done
	fi
fi
if test -z "$QTDOCDIR"  || test \! -d "$QTDOCDIR" ; then
	echo "* QTDOCDIR could not be guessed, or not set correctly."
	QTDOCDIR=/vol/nonexistent
fi

top_srcdir="$1"
module_name=`basename "$top_srcdir"`
subdir="$2"
test "x." = "x$subdir" && subdir=""

if test -z "$subdir" ; then
	if test "x$recurse" = "x1" ; then
		rm -rf "$module_name"-apidocs
		mkdir "$module_name"-apidocs
	fi
	cd "$module_name"-apidocs || exit 1

	# Extract some constants from the top-level files and
	# store them in Makefile.in for later reference.
	eval `grep 'VERSION="' "$top_srcdir/admin/cvs.sh"`
	echo "PROJECT_NUMBER = $VERSION" > Doxyfile.in
	grep ^KDE_INIT_DOXYGEN "$top_srcdir/configure.in.in" | \
		sed -e 's+[^[]*\[\([^]]*\)+PROJECT_NAME = "\1"+' \
			-e 's+].*++' >> Doxyfile.in

	top_builddir="."
	srcdir="$1"
	subdir="."
else
	cd "$module_name"-apidocs || exit 1
	srcdir="$top_srcdir/$subdir"
	subdirname=`basename "$subdir"`
	top_builddir=`perl -e '$foo="'$3'"; $foo+="/" unless $foo=~/\/$/; $foo=~s+[^/].*+../+; $foo=~s+/$++; print $foo;'`
fi

extract_line()
{
	pattern=`echo "$1" | tr + .`
	grep "^$1" "$srcdir/Makefile.am" | \
		sed -e "s+$pattern.*=\s*++"
}

### Add HTML header, footer, CSS tags to Doxyfile.
### Assumes $subdir is set. Argument is a string
### to stick in front of the file if needed.
apidox_htmlfiles()
{
	dox_header="$top_srcdir/doc/api/$1header.html" ; \
	dox_footer="$top_srcdir/doc/api/$1footer.html" ; \
	dox_css="$top_srcdir/doc/api/doxygen.css" ; \
	test -f "$dox_header" || dox_header="$top_srcdir/admin/Dox-$1header.html" ; \
	test -f "$dox_footer" || dox_footer="$top_srcdir/admin/Dox-$1footer.html" ; \
	test -f "$dox_css" || dox_css="$top_srcdir/admin/doxygen.css"

	echo "HTML_HEADER            = $dox_header" >> "$subdir/Doxyfile" ; \
	echo "HTML_FOOTER            = $dox_footer" >> "$subdir/Doxyfile" ; \
	echo "HTML_STYLESHEET        = $dox_css" >> "$subdir/Doxyfile"
}

apidox_toplevel()
{
	echo "*** Creating API documentation main page"
	if test -f $top_srcdir/admin/Doxyfile.global ; then
		cp $top_srcdir/admin/Doxyfile.global Doxyfile
	else
		cp $top_srcdir/doc/api/Doxyfile.pim Doxyfile
	fi


	cat "$top_builddir/Doxyfile.in" >> Doxyfile
	grep ^DOXYGEN "$top_srcdir/Makefile.am.in" >> Doxyfile

	echo "INPUT                  = $top_srcdir" >> Doxyfile ; \
	echo "OUTPUT_DIRECTORY       = $top_builddir" >> Doxyfile ; \
	echo "FILE_PATTERNS          = *.dox" >> Doxyfile ; \
	echo "RECURSIVE              = NO" >> Doxyfile ; \
	echo "ALPHABETICAL_INDEX     = NO" >> Doxyfile ; \
	echo "HTML_OUTPUT            = ." >> Doxyfile ; \
	apidox_htmlfiles "main"

	doxygen Doxyfile
	rm -f Doxyfile

	( cd "$top_srcdir" && grep -l ^include.*Doxyfile.am `find . -name Makefile.am` ) | sed -e 's+/Makefile.am$++' -e 's+^\./++' | sort > subdirs.in
	for i in `cat subdirs.in`
	do
		test "x." = "x$i" && continue;

		dir=`dirname "$i"`
		file=`basename "$i"`
		if test "x." = "x$dir" ; then
			dir=""
		else
			dir="$dir/"
		fi
		indent=`echo "$dir" | sed -e 's+[^/]*/+\&nbsp;\&nbsp;+g' | sed -e 's+&+\\\&+g'`
		echo "<li>$indent<a href=\"@topdir@/$dir$file/html/index.html\">$file</a></li>"
	done > subdirs

	dox_index="$top_srcdir/doc/api/doxyndex.sh" ; \
	test -f "$dox_index" || dox_index="$top_srcdir/admin/Doxyndex.sh" ; \
	sh "$dox_index" "$top_builddir" .
}

apidox_subdir()
{
	echo "*** Creating apidox in $subdir"
	dox_file="$top_srcdir/doc/api/Doxyfile.pim" ; \
	test -f "$dox_file" || dox_file="$top_srcdir/admin/Doxyfile.global" ; \
	cp "$dox_file" "$subdir/Doxyfile"



	cat "Doxyfile.in" >> "$subdir/Doxyfile"

	echo "PROJECT_NAME           = \"$subdir\"" >> "$subdir/Doxyfile"
	echo "INPUT                  = $srcdir" >> "$subdir/Doxyfile"
	echo "IMAGE_PATH             = $top_srcdir/doc/api" >> "$subdir/Doxyfile"
	echo "OUTPUT_DIRECTORY       = ." >> "$subdir/Doxyfile"
	echo "HTML_OUTPUT            = $subdir/html" >> "$subdir/Doxyfile"
	echo "GENERATE_TAGFILE       = $subdir/$subdirname.tag" >> "$subdir/Doxyfile"

	apidox_htmlfiles ""

	excludes=`extract_line DOXYGEN_EXCLUDE`
	if test -n "$excludes"; then
		patterns=""
		dirs=""
		for item in `echo "$excludes"`; do
			if test -d "$subdir/$item"; then
				dirs="$dirs $subdir/$item/"
			else
				patterns="$patterns $item"
			fi
		done
		echo "EXCLUDE_PATTERNS      += $patterns" >> "$subdir/Doxyfile"
		echo "EXCLUDE               += $dirs" >> "$subdir/Doxyfile"
	fi

	echo "TAGFILES = \\" >> "$subdir/Doxyfile"
	## For now, don't support \ continued references lines
	tags=`extract_line DOXYGEN_REFERENCES`
	for i in $tags qt ; do
		tagsubdir=`dirname $i` ; tag=`basename $i`
		tagpath=""

		# Find location of tag file
		if test -f "$tagsubdir/$tag/$tag.tag" ; then
			file="$tagsubdir/$tag/$tag.tag"
			loc="$tagsubdir/$tag/html"
		else
			file=`ls -1 ../*-apidocs/$tagsubdir/$tag/$tag.tag`
			loc=`echo "$file" | sed -e "s/$tag.tag\$/html/"`
		fi
		if test "$tag" = "qt" ; then
			echo "  $file=$QTDOCDIR" >> "$subdir/Doxyfile"
		else
			test -n "$file" && \
				echo "  $file=../$top_builddir/$loc \\" >> "$subdir/Doxyfile"
		fi
	done

	doxygen "$subdir/Doxyfile"
	dox_index="$top_srcdir/doc/api/doxyndex.sh"
	test -f "$dox_index" || dox_index="$top_srcdir/admin/Doxyndex.sh"
	sh "$dox_index" "." "$subdir/html"
}

create_installdox()
{
# Fix up the installdox script so it accepts empty args
#
# This code is copied from the installdox generated by Doxygen,
# copyright by Dimitri van Heesch and released under the GPL.
# This does a _slow_ update of the dox, because it loops
# over the given substitutions instead of assuming all the
# needed ones are given.
#
cat <<\EOF
#! /usr/bin/env perl

%subst = () ;
$quiet   = 0;

if (open(F,"search.cfg"))
{
  $_=<F> ; s/[ \t\n]*$//g ; $subst{"_doc"} = $_;
  $_=<F> ; s/[ \t\n]*$//g ; $subst{"_cgi"} = $_;
}

while ( @ARGV ) {
  $_ = shift @ARGV;
  if ( s/^-// ) {
    if ( /^l(.*)/ ) {
      $v = ($1 eq "") ? shift @ARGV : $1;
      ($v =~ /\/$/) || ($v .= "/");
      $_ = $v;
      if ( /(.+)\@(.+)/ ) {
          $subst{$1} = $2;
      } else {
        print STDERR "Argument $_ is invalid for option -l\n";
        &usage();
      }
    }
    elsif ( /^q/ ) {
      $quiet = 1;
    }
    elsif ( /^\?|^h/ ) {
      &usage();
    }
    else {
      print STDERR "Illegal option -$_\n";
      &usage();
    }
  }
  else {
    push (@files, $_ );
  }
}


if ( ! @files ) {
  if (opendir(D,".")) {
    foreach $file ( readdir(D) ) {
      $match = ".html";
      next if ( $file =~ /^\.\.?$/ );
      ($file =~ /$match/) && (push @files, $file);
      ($file =~ "tree.js") && (push @files, $file);
    }
    closedir(D);
  }
}

if ( ! @files ) {
  print STDERR "Warning: No input files given and none found!\n";
}

foreach $f (@files)
{
  if ( ! $quiet ) {
    print "Editing: $f...\n";
  }
  $oldf = $f;
  $f   .= ".bak";
  unless (rename $oldf,$f) {
    print STDERR "Error: cannot rename file $oldf\n";
    exit 1;
  }
  if (open(F,"<$f")) {
    unless (open(G,">$oldf")) {
      print STDERR "Error: opening file $oldf for writing\n";
      exit 1;
    }
    if ($oldf ne "tree.js") {
      while (<F>) {
	foreach $sub (keys %subst) {
          s/doxygen\=\"$sub\:([^ \"\t\>\<]*)\" (href|src)=\"\1/doxygen\=\"$sub:$subst{$sub}\" \2=\"$subst{$sub}/g;
          print G "$_";
	}
      }
    }
    else {
      while (<F>) {
	foreach $sub (keys %subst) {
          s/\"$sub\:([^ \"\t\>\<]*)\", \"\1/\"$sub:$subst{$sub}\" ,\"$subst{$sub}/g;
          print G "$_";
	}
      }
    }
  } 
  else {
    print STDERR "Warning file $f does not exist\n";
  }
  unlink $f;
}

sub usage {
  print STDERR "Usage: installdox [options] [html-file [html-file ...]]\n";
  print STDERR "Options:\n";
  print STDERR "     -l tagfile\@linkName   tag file + URL or directory \n";
  print STDERR "     -q                    Quiet mode\n\n";
  exit 1;
}
EOF
}


if test "x." = "x$top_builddir" ; then
	apidox_toplevel
	create_installdox > installdox-slow
	if test "x$recurse" = "x1" ; then
		if test "x$module_name" = "xkdelibs" ; then
			# Special case: create a qt tag file.
			echo "*** Creating a tag file for the Qt library:"
			mkdir qt
			doxytag -t qt/qt.tag "$QTDOCDIR" > /dev/null 2>&1
		fi

		for i in `cat subdirs.in`
		do
			if test "x$i" = "x." ; then
				continue
			fi

			subdir="$i"
			srcdir="$top_srcdir/$subdir"
			subdirname=`basename "$subdir"`
			mkdir -p "$subdir"
			top_builddir=`echo "/$subdir" | sed -e 's+/[^/]*+../+g'`
			apidox_subdir
		done
	fi
else
	apidox_subdir
fi










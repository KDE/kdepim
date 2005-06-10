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
# if that is not set, $QTDIR, or otherwise guessed. You may explicitly
# set the location of a pre-generated tag file with $QTDOCTAG. One
# typical approach might be:
#
# QTDOCTAG=$QTDIR/doc/qt.tag QTDOCDIR=http://doc.trolltech.com/3.3/
#
# Finally, there is a --no-recurse option for top-level generation
# that avoids generating all the subdirectories as well. It also
# suppresses cleaning up (rm -rf) of the dox direction beforehand.


### There is only one option, so handle it in a clumsy way
recurse=1
if test "x--no-recurse" = "x$1" ; then
	shift
	recurse=0
fi

### Sanity check the mandatory "top srcdir" argument.
if test -z "$1" ; then
	echo "Usage: doxygen.sh <top_srcdir>"
	exit 1
fi
if ! test -d "$1" ; then
	echo "top_srcdir ($1) is not a directory."
	exit 1
fi

### Sanity check and guess QTDOCDIR.
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
	echo "*** QTDOCDIR could not be guessed, or not set correctly."
	if test -z "$QTDOCTAG" ; then
		echo "* QTDOCDIR set to \"\""
		QTDOCDIR=""
	else
		echo "* But I'll use $QTDOCDIR anyway because of QTDOCTAG."
	fi
fi

### Get the "top srcdir", also its name, and handle the case that subdir "."
### is given (which would be top_srcdir then, so it's equal to none-given
### but no recursion either).
###
top_srcdir="$1"
module_name=`basename "$top_srcdir"`
subdir="$2"
if test "x." = "x$subdir" ; then
	subdir=""
	recurse=0
fi

### If we're making the top subdir, create the structure
### for the apidox and initialize it. Otherwise, just use the
### structure assumed to be there.
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

### Read a single line (TODO: support \ continuations) from the Makefile.am.
### Used to extract variable assignments from it.
extract_line()
{
	pattern=`echo "$1" | tr + .`
	grep "^$1" "$srcdir/Makefile.am" | \
		sed -e "s+$pattern.*=\s*++"
}

### Handle the COMPILE_{FIRST,LAST,BEFORE,AFTER} part of Makefile.am
### in the toplevel. Copied from admin/cvs.sh. Licence presumed LGPL).
create_subdirs()
{
echo "* Sorting top-level subdirs"
dirs=
idirs=
if test -f "$top_srcdir/inst-apps"; then
   idirs=`cat "$top_srcdir/"inst-apps`
else
   idirs=`cd "$top_srcdir" && ls -1 | sort`
fi

compilefirst=`sed -ne 's#^COMPILE_FIRST[ ]*=[ ]*##p' "$top_srcdir/"Makefile.am.in | head -n 1`
compilelast=`sed -ne 's#^COMPILE_LAST[ ]*=[ ]*##p' "$top_srcdir/"Makefile.am.in | head -n 1`
for i in $idirs; do
    if test -f "$top_srcdir/"$i/Makefile.am; then
       case " $compilefirst $compilelast " in
         *" $i "*) ;;
         *) dirs="$dirs $i"
       esac
    fi
done

: > ./_SUBDIRS

for d in $compilefirst; do
   echo $d >> ./_SUBDIRS
done

(for d in $dirs; do 
   list=`sed -ne "s#^COMPILE_BEFORE_$d""[ ]*=[ ]*##p" "$top_srcdir/"Makefile.am.in | head -n 1`
   for s in $list; do
      echo $s $d
   done
   list=`sed -ne "s#^COMPILE_AFTER_$d""[ ]*=[ ]*##p" "$top_srcdir/"Makefile.am.in | head -n 1`
   for s in $list; do
      echo $d $s
   done
   echo $d $d
done ) | tsort >> ./_SUBDIRS

for d in $compilelast; do
   echo $d >> ./_SUBDIRS
done

test -r _SUBDIRS && mv _SUBDIRS subdirs.top || true
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

### Handle the Doxygen processing of a toplevel directory.
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

### Handle the Doxygen processing of a non-toplevel directory.
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
			file=`ls -1 ../*-apidocs/$tagsubdir/$tag/$tag.tag 2> /dev/null`
			loc=`echo "$file" | sed -e "s/$tag.tag\$/html/"`

			# If the tag file doesn't exist yet, but should
			# because we have the right dirs here, queue
			# this directory for re-processing later.
			if test -z "$file" && test -d "$top_srcdir/$tagsubdir/$tag" ; then
				echo "* Need to re-process $subdir for tag $i"
				echo "$subdir" >> "subdirs.later"
			fi
		fi
		if test "$tag" = "qt" ; then
			if test -z "$QTDOCDIR" ; then
				echo "  $file" >> "$subdir/Doxyfile"
			else
				echo "  $file=$QTDOCDIR" >> "$subdir/Doxyfile"
			fi
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

### Run a given subdir by setting up global variables first.
do_subdir()
{
	subdir="$i"
	srcdir="$top_srcdir/$subdir"
	subdirname=`basename "$subdir"`
	mkdir -p "$subdir"
	top_builddir=`echo "/$subdir" | sed -e 's+/[^/]*+../+g'`
	apidox_subdir
}


### Create installdox-slow in the toplevel
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
	create_subdirs
	create_installdox > installdox-slow
	if test "x$recurse" = "x1" ; then
		if test "x$module_name" = "xkdelibs" ; then
			if test -z "$QTDOCTAG" && test -d "$QTDOCDIR" ; then
				# Special case: create a qt tag file.
				echo "*** Creating a tag file for the Qt library:"
				mkdir qt
				doxytag -t qt/qt.tag "$QTDOCDIR" > /dev/null 2>&1
			fi
		fi
		if test -n "$QTDOCTAG" && test -r "$QTDOCTAG" ; then
			echo "*** Copying tag file for the Qt library:"
			mkdir qt
			cp "$QTDOCTAG" qt/qt.tag
		fi

		# Here's a queue of dirs to re-process later when 
		# all the rest have been done already.
		> subdirs.later

		# subdirs.top lists _all_ subdirs of top in the order they 
		# should be handled; subdirs.in lists those dirs that contain
		# dox. So the intersection of the two is the ordered list
		# of top-level subdirs that contain dox.
		#
		# subdirs.top also doesn't contain ".", so that special
		# case can be ignored in the loop.

		
		(
		for i in `cat subdirs.top`
		do
			if test "x$i" = "x." ; then
				continue
			fi
			# Calculate intersection of this element and the
			# set of dox dirs.
			if grep "^$i\$" subdirs.in > /dev/null 2>&1 ; then
				echo "$i"
				mkdir -p "$i" 2> /dev/null

				# Handle the subdirs of this one
				for j in `grep "$i/" subdirs.in`
				do
					echo "$j"
					mkdir -p "$j" 2> /dev/null
				done
			fi
		done

		# Now we still need to handle whatever is left
		for i in `cat subdirs.in`
		do
			test -d "$i" || echo "$i"
			mkdir -p "$i" 2> /dev/null
		done
		) > subdirs.sort
		for i in `cat subdirs.sort`
		do
			do_subdir "$i"
		done

		if test -s "subdirs.later" ; then
			sort subdirs.later | uniq > subdirs.sort
			for i in `cat subdirs.sort`
			do
				: > subdirs.later
				echo "*** Reprocessing $i"
				do_subdir "$i"
				test -s "subdirs.later" && echo "* Some tag files were still not found."
			done
		fi

	fi
else
	apidox_subdir
fi









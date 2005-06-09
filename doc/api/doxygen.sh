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
if test -z "$QTDOCDIR"  || test -d "$QTDOCDIR" ; then
	echo "* QTDOCDIR could not be guessed, or not set correctly."
	QTDOCDIR=/vol/nonexistent
fi

top_srcdir="$1"
module_name=`basename "$top_srcdir"`
subdir="$2"
test "x." = "x$subdir" && subdir=""

if test -z "$subdir" ; then
	rm -rf "$module_name"-apidocs
	mkdir "$module_name"-apidocs
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
### 		if test -n "$(DOXYGEN_EXCLUDE)"; then \
### 			patterns= ;\
### 			dirs= ;\
### 			for item in `echo "$(DOXYGEN_EXCLUDE)"`; do \
### 				if test -d "$(srcdir)/$$item"; then \
### 					dirs="$$dirs $(srcdir)/$$item/" ;\
### 				else \
### 					patterns="$$patterns $$item" ;\
### 				fi ;\
### 			done ;\
### 			echo "EXCLUDE_PATTERNS      += $$patterns" >> Doxyfile; \
### 			echo "EXCLUDE               += $$dirs" >> Doxyfile ;\
### 		fi ;\
	echo "TAGFILES = \\" >> "$subdir/Doxyfile"
	## For now, don't support \ continued references lines
	tags=`grep ^DOXYGEN_REFERENCES "$srcdir/Makefile.am" | \
		sed -e 's+DOXYGEN.*=\s*++'`
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


if test "x." = "x$top_builddir" ; then
	apidox_toplevel
	if test "x$recurse" = "x1" ; then
		if test "x$module_name" = "xkdelibs" ; then
			# Special case: create a qt tag file.
			echo "*** Creating a tag file for the Qt library:"
			mkdir qt
			doxytag -t qt/qt.tag "$QTDOCDIR"
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










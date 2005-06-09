#! /bin/sh

# A shell script that builds dox without all the tedious mucking about with
# autoconf and configure. Run it in the "top builddir" with one argument,
# the "top srcdir". Something like this:
#
# cd /mnt/build/kdepim
# sh /mnt/src/kdepim/doc/api/doxygen.sh /mnt/src/kdepim

test -z "$MAKE" && MAKE=gmake

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

top_srcdir="$1"
subdir="$2"
test "x." = "x$subdir" && subdir=""

if test -z "$subdir" ; then
	rm -rf apidocs
	mkdir apidocs
	cd apidocs || exit 1

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
	srcdir="$top_srcdir/$subdir"
	subdirname=`basename "$subdir"`
	top_builddir="$3"
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

	echo "PROJECT_NAME           = \"$subdirname\"" >> "$subdir/Doxyfile"
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
### 		echo "TAGFILES = \\" >> Doxyfile; \
### 		tags='$(DOXYGEN_REFERENCES) qt'; for tag in $$tags; do \
### 			tagsubdir=`dirname $$tag` ; tag=`basename $$tag` ; \
### 			tagpath= ;\
### 			path="$(top_builddir)/../$$tagsubdir/$$tag" ;\
### 			if test -f $(top_builddir)/apidocs/$$tagsubdir/$$tag/$$tag.tag; then \
### 				tagpath="$(top_builddir)/apidocs/$$tagsubdir/$$tag/$$tag.tag" ;\
### 			else \
### 				tagpath=`ls -1 $(kde_htmldir)/en/*-apidocs/$$tagsubdir/$$tag/$$tag.tag 2> /dev/null` ;\
### 				if test -n "$$tagpath"; then \
### 					path=`echo $$tagpath | sed -e "s,.*/\([^/]*-apidocs\)/$$tagsubdir/$$tag/$$tag.tag,$(top_builddir)/../../\1/$$tag,"` ;\
### 				fi ;\
### 			fi ;\
### 			if test "$$tag" = qt; then \
### 				echo "	$$tagpath=$(QTDOCDIR)" >> Doxyfile ;\
### 			else if test -n "$$tagpath"; then \
### 				echo "$$tagpath=$$path/html \\" >> Doxyfile ;\
### 				fi ;\
### 			fi ;\
### 		done
	doxygen "$subdir/Doxyfile"
	dox_index="$top_srcdir/doc/api/doxyndex.sh"
	test -f "$dox_index" || dox_index="$top_srcdir/admin/Doxyndex.sh"
	sh "$dox_index" "." "$subdir/html"
}


if test "x." = "x$top_builddir" ; then
	apidox_toplevel
	if test "x$recurse" = "x1" ; then
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










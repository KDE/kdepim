#! /bin/sh
#
# A shell script to post-process doxy-generated files; the purpose
# is to make the menu on the left in the file match the actually
# generated files (ie. leave out namespaces if there are none).
#
# Usage: doxyndex.sh <html-output-directory>

echo "Postprocessing files in $1"

MENU="<ul>"

# This is a list of pairs, with / separators so we can use basename
# and dirname (a crude shell hack) to split them into parts. For
# each, if the file part exists (as a html file) tack it onto the
# MENU variable as a <li> with link.
for i in "Main Page/index" \
	"Namespace List/namespaces" \
	"Class Hierarchy/hierarchy" \
	"Alphabetical List/classes" \
	"Class List/annotated" \
	"File List/files" \
	"Namespace Members/namespacemembers" \
	"Class Members/functions"
do
	NAME=`dirname "$i"`
	FILE=`basename "$i"`
	test -f "$1/$FILE.html" && MENU="$MENU<li><a href=\"$FILE.html\">$NAME</a></li>"
done

MENU="$MENU</ul>"

# Now substitute in the MENU in every file. This depends
# on HTML_HEADER (ie. header.html) containing the <!-- menu --> comment.
for i in "$1"/*.html
do
	sed -e "s+<!-- menu -->+$MENU+" < "$i" > "$i.new"
	mv "$i.new" "$i"
done


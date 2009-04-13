#! /bin/sh
LIST=`find . -name \*.cpp -o -name \*.cxx`
if test -n "$LIST"; then
$XGETTEXT $LIST -o $podir/kmailcvt.pot
fi

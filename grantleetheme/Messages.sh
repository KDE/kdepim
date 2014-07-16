#! /bin/sh
$XGETTEXT `find . -name '*.cpp' -o -name '*.h'` -o $podir/libgrantleetheme.pot
rm -f rc.cpp

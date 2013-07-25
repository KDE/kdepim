#! /bin/sh
$XGETTEXT `find . -name '*.cpp' -o -name '*.h' | grep -v '/tests/' |egrep -v '/headerthemeeditor/'` -o $podir/libgrantleetheme.pot
rm -f rc.cpp

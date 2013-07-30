#! /bin/sh
$XGETTEXT `find . -name '*.cpp' -o -name '*.h' | grep -v '/tests/'` -o $podir/libgrantleethemeeditor.pot
rm -f rc.cpp

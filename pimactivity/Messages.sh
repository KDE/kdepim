#! /bin/sh
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/'` -o $podir/libpimactivity.pot
rm -f rc.cpp

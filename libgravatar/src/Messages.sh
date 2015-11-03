#! /bin/sh
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' ` -o $podir/libgravatar.pot
rm -f rc.cpp

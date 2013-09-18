#! /bin/sh
$XGETTEXT `find . -name '*.cpp' -o -name '*.h'` -o $podir/libkaddressbookgrantlee.pot
rm -f rc.cpp

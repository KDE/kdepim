#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/backupmail.pot
rm -f rc.cpp

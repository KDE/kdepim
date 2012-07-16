#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/importwizard.pot
rm -f rc.cpp

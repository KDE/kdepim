#! /bin/sh
$EXTRACTRC ui/*.ui  >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/importwizard.pot
rm -f rc.cpp

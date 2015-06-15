#! /bin/sh
$EXTRACTRC `find -name '*.ui'`  >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/importwizard.pot
rm -f rc.cpp

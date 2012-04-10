#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp || exit 11
$XGETTEXT rc.cpp *.cpp -o $podir/ktnef.pot

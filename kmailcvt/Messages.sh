#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/kmailcvt.pot

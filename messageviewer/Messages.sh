#! /bin/sh
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT `find . -name '*.cpp'` -o $podir/libmessageviewer.pot

#! /bin/sh
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp soap/*.cpp -o $podir/kres_groupwise.pot

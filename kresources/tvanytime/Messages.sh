#! /bin/sh
$EXTRACTRC `find . -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/kres_tvanytime.pot

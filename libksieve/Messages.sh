#! /bin/sh
$EXTRACTRC `find . -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp | grep -v '/tests/'` -o $podir/libksieve.pot
rm -f rc.cpp

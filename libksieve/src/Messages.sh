#! /bin/sh
$EXTRACTRC `find . -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp | grep -v '/tests/' | grep -v '/autotests/'` -o $podir/libksieve.pot
rm -f rc.cpp

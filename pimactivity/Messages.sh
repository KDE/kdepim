#! /bin/sh
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/' |grep -v '/kcm/'` -o $podir/libpimactivity.pot
rm -f rc.cpp

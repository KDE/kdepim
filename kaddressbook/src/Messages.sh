#! /bin/sh
$EXTRACTRC `find . -name \*.ui` `find . -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find -name \*.cpp -o -name \*.h` -o $podir/kaddressbook.pot
rm -f rc.cpp

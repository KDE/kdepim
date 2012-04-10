#! /bin/sh
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h" | grep -v '/tests/'` -o $podir/ktnef.pot
rm -f rc.cpp

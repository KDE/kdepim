#! /bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/autotests/'` -o $podir/libpimcommon.pot
rm -f rc.cpp

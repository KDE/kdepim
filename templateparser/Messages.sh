#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/libtemplateparser.pot
rm -f rc.cpp

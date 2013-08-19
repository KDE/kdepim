#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.cpp' -o -name '*.h'` -o $podir/headerthemeeditor.pot
rm -f rc.cpp

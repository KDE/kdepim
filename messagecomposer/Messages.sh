#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg' -or -name '*.kcfg.cmake'` >> rc.cpp || exit 11
$XGETTEXT autocorrection/*.cpp *.cpp -o $podir/libmessagecomposer.pot
rm -f rc.cpp

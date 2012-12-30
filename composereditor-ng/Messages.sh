#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.kcfg' >> rc.cpp || exit 11
$XGETTEXT *.h *.cpp -o | grep -v "/tests" $podir/libcomposereditorng.pot
rm -f rc.cpp

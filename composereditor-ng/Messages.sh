#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.kcfg' >> rc.cpp || exit 11
$XGETTEXT *.h *.cpp -o $podir/libcomposereditorng.pot
rm -f rc.cpp

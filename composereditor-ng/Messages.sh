#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -o -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/'` -o $podir/libcomposereditorng.pot
rm -f rc.cpp

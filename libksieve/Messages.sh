#! /bin/sh
$EXTRACTRC `find . -name '*.kcfg' >> rc.cpp || exit 11
$XGETTEXT shared/*.cpp parser/*.cpp impl/*.h ksieve/*.h ksieveui/*.cpp -o $podir/libksieve.pot
rm -f rc.cpp

#! /bin/sh
$EXTRACTRC --ignore-no-input `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg' -or -name '*.kcfg.cmake'` >> rc.cpp || exit 11
$XGETTEXT -ktranslate `find -name '*.cpp' -o -name '*.h' -o -name '*.qml'` -o $podir/kaddressbook-mobile.pot
rm -f rc.cpp

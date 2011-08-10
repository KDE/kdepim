#! /bin/sh
$EXTRACTRC --ignore-no-input `find . \( -name '*.ui' -or -name '*.rc' -or -name '*.kcfg' -or -name '*.kcfg.cmake' \) -and -not -name '*-mobile.rc'` >> rc.cpp || exit 11
$XGETTEXT -ktranslate `find -name '*.cpp' -o -name '*.h'` -o $podir/kmail-mobile.pot
$XGETTEXT -ktranslate `find . -name '*.qml'` -j -L Java -o $podir/kmail-mobile.pot
rm -f rc.cpp

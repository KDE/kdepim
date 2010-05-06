#! /bin/sh
$EXTRACTRC --ignore-no-input `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg' -or -name '*.kcfg.cmake'` >> rc.cpp || exit 11
# TODO: enable once the QML extraction support has been committed to l10n-kde4/scripts
#$EXTRACTQML `find . -name '*.qml'` >> rc.cpp || exit 11
$XGETTEXT -ktranslate `find -name '*.cpp' -o -name '*.h'` -o $podir/kjots-mobile.pot
rm -f rc.cpp

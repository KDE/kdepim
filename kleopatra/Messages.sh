#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/' | grep -v '/kwatchgnupg/'` -o $podir/kleopatra.pot
$XGETTEXT -ktranslate `find . -name '*.qml'` -j -L Java -o $podir/kleopatra.pot

#! /bin/sh
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT `find . \( ! -path "./akonadi/*" \) -a \( -name "*.cpp" -o -name "*.h" \)` -o $podir/kalarm.pot

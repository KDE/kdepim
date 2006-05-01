#! /bin/sh
$EXTRACTRC gui/*.rc gui/*.ui >> rc.cpp || exit 11
$XGETTEXT rc.cpp gui/*.cpp lib/*.cpp -o $podir/ktnef.pot

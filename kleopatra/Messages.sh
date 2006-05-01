#! /bin/sh
$EXTRACTRC conf/*.ui >> rc.cpp || exit 11
$XGETTEXT conf/*.cpp *.cpp *.h -o $podir/kleopatra.pot

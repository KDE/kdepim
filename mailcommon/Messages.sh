#! /bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 11
$XGETTEXT *.h *.cpp filter/*.cpp filter/*.h -o $podir/libmailcommon.pot
rm -f rc.cpp

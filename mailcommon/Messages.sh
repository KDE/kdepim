#! /bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 11
$XGETTEXT *.h *.cpp -o $podir/libmailcommon.pot
rm -f rc.cpp

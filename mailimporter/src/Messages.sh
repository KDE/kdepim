#! /bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/libmailimporter.pot
rm -f rc.cpp

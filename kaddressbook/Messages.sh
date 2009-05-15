#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find -name \*.cpp -o -name \*.h` -o $podir/kcontactmanager.pot
rm -f rc.cpp

#! /bin/sh
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/archivemailagent.pot
rm -f rc.cpp

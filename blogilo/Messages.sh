#! /usr/bin/env bash
$EXTRACTRC `find -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find -name \*.ui` >> rc.cpp || exit 12
$EXTRACTRC `find -name \*.kcfg` >> rc.cpp
$XGETTEXT `find -name \*.cpp -o -name \*.h` rc.cpp -o $podir/blogilo.pot
rm -f rc.cpp

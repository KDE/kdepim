#!/bin/sh
$EXTRACTRC *.kcfg >> rc.cpp
$XGETTEXT *.h *.cpp -o $podir/calendarsupport.pot

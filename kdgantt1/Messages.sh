#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT_QT *.cpp *.h -o $podir/kdgantt1.pot

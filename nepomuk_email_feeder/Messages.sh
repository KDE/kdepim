#! /bin/sh
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT `find -name '*.cpp'` -o $podir/akonadi_nepomuk_email_feeder.pot

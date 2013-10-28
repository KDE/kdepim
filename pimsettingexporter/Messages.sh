#! /bin/sh
$XGETTEXT `find -name '*.cpp' | grep -v '/tests/'` -o $podir/pimsettingexporter.pot


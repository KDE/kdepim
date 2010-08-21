#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma_applet_akonotes_note.pot

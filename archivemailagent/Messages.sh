#! /bin/sh
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/akonadi_archivemail_agent.pot
rm -f rc.cpp

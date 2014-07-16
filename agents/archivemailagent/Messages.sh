#! /bin/sh
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/'` -o $podir/akonadi_archivemail_agent.pot
rm -f rc.cpp

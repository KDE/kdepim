#! /bin/sh
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/akonadi_folderagent_agent.pot

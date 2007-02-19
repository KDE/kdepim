#! /bin/sh
$EXTRACTRC --context="Instant messaging" editors/im*.ui >> rc.cpp || exit 11
$EXTRACTRC printing/*.ui >> rc.cpp || exit 11
$EXTRACTRC */*.kcfg >> rc.cpp || exit 11
$XGETTEXT *.h *.cpp common/*.cpp editors/*.cpp features/*.cpp printing/*.cpp views/*.cpp xxport/*.cpp interfaces/*.h -o $podir/kaddressbook.pot

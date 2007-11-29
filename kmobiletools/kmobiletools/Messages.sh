#$PREPARETIPS > tips.cpp

ABOUT_RCS="`find about -name \"*.ui\" -o -name \"*.rc\" -o -name \"*.kcfg\"`"
LIBKMOBILETOOLS_RCS="`find libkmobiletools -name \"*.ui\" -o -name \"*.rc\" -o -name \"*.kcfg\"`"
LIBKMOBILETOOLSENGINEUI_RCS="`find engines/libkmobiletoolsengineui -name \"*.ui\" -o -name \"*.rc\" -o -name \"*.kcfg\"`"
MAINPART_RCS="`find mainpart -name \"*.ui\" -o -name \"*.rc\" -o -name \"*.kcfg\"`"

ABOUT_SRCS="`find about -name \"*.cpp\" -o -name \"*.h\"`"
LIBKMOBILETOOLS_SRCS="`find libkmobiletools -name \"*.cpp\" -o -name \"*.h\"`"
LIBKMOBILETOOLSENGINEUI_SRCS="`find engines/libkmobiletoolsengineui -name \"*.cpp\" -o -name \"*.h\"`"
MAINPART_SRCS="`find mainpart -name \"*.cpp\" -o -name \"*.h\"`"

KMOBILETOOLS_RCS="$ABOUT_RCS $LIBKMOBILETOOLS_RCS $LIBKMOBILETOOLSENGINEUI_RCS $MAINPART_RCS"
KMOBILETOOLS_SRCS="$ABOUT_SRCS $LIBKMOBILETOOLS_SRCS $LIBKMOBILETOOLSENGINEUI_SRCS $MAINPART_SRCS"

$EXTRACTRC *.rc *.ui *.kcfg $KMOBILETOOLS_RCS >> rc.cpp
$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t *.cpp *.h $KMOBILETOOLS_SRCS -o $podir/kmobiletools.pot
rm -f tips.cpp rc.cpp

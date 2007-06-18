RESOURCE_FILES="`find -name \"*.ui\" -o -name \"*.rc\" -o -name \"*.kcfg\"`"

$EXTRACTRC $RESOURCE_FILES > rc.cpp

SOURCE_FILES="`find -name \"*.cpp\" -o -name \"*.h\"`"

$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t $SOURCE_FILES -o $podir/kmobiletools_at_engine.pot
rm -f tips.cpp rc.cpp

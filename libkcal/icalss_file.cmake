# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering.
# It is required to make the combined header icalss.h properly.
set(COMBINEDHEADERSICALSS
   ${TOP}/libkcal/libical/src/libicalss/icalgauge.h
   ${TOP}/libkcal/libical/src/libicalss/icalset.h
   ${TOP}/libkcal/libical/src/libicalss/icalcluster.h
   ${TOP}/libkcal/libical/src/libicalss/icalfileset.h
   ${TOP}/libkcal/libical/src/libicalss/icaldirset.h
   ${TOP}/libkcal/libical/src/libicalss/icalcalendar.h
   ${TOP}/libkcal/libical/src/libicalss/icalclassify.h
   ${TOP}/libkcal/libical/src/libicalss/icalspanlist.h
   ${TOP}/libkcal/libical/src/libicalss/icalmessage.h
)

FILE(WRITE  ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "extern \"C\" {\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")
FILE(APPEND ${KDE_FILE_H_FILE} "/*\n")
FILE(APPEND ${KDE_FILE_H_FILE} " $Id$\n")
FILE(APPEND ${KDE_FILE_H_FILE} "*/\n")

foreach (_current_FILE ${COMBINEDHEADERSICALSS})
   FILE(READ ${_current_FILE} _contents)
   STRING(REGEX REPLACE "#include *\"ical.*\\.h\"" "" _contents "${_contents}" )
   STRING(REGEX REPLACE "#include *\"pvl\\.h\"" "" _contents "${_contents}" )
   #STRING(REGEX REPLACE "\$$(Id|Locker): .+\$$" "" _contents "${_contents}" )
   FILE(APPEND ${KDE_FILE_H_FILE} "${_contents}")
endforeach (_current_FILE)

FILE(APPEND ${KDE_FILE_H_FILE} "\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "}\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")

# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering. It
# is required to make the combined header ical.h properly
set(COMBINEDHEADERSICAL
   ${TOP}/libkcal/libical/src/libical/icalversion.h
   ${TOP}/libkcal/libical/src/libical/icaltime.h
   ${TOP}/libkcal/libical/src/libical/icalduration.h
   ${TOP}/libkcal/libical/src/libical/icalperiod.h
   ${TOP}/libkcal/libical/src/libical/icalenums.h
   ${TOP}/libkcal/libical/src/libical/icaltypes.h
   ${TOP}/libkcal/libical/src/libical/icalrecur.h
   ${TOP}/libkcal/libical/src/libical/icalattach.h
   ${TOP}/libkcal/libical/src/libical/icalderivedvalue.h
   ${TOP}/libkcal/libical/src/libical/icalderivedparameter.h
   ${TOP}/libkcal/libical/src/libical/icalvalue.h
   ${TOP}/libkcal/libical/src/libical/icalparameter.h
   ${TOP}/libkcal/libical/src/libical/icalderivedproperty.h
   ${TOP}/libkcal/libical/src/libical/icalproperty.h
   ${TOP}/libkcal/libical/src/libical/pvl.h
   ${TOP}/libkcal/libical/src/libical/icalarray.h
   ${TOP}/libkcal/libical/src/libical/icalcomponent.h
   ${TOP}/libkcal/libical/src/libical/icaltimezone.h
   ${TOP}/libkcal/libical/src/libical/icalparser.h
   ${TOP}/libkcal/libical/src/libical/icalmemory.h
   ${TOP}/libkcal/libical/src/libical/icalerror.h
   ${TOP}/libkcal/libical/src/libical/icalrestriction.h
   ${TOP}/libkcal/libical/src/libical/sspm.h
   ${TOP}/libkcal/libical/src/libical/icalmime.h
   ${TOP}/libkcal/libical/src/libical/icallangbind.h
)

FILE(WRITE  ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "extern \"C\" {\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")
FILE(APPEND ${KDE_FILE_H_FILE} "/*\n")
FILE(APPEND ${KDE_FILE_H_FILE} " $Id$\n")
FILE(APPEND ${KDE_FILE_H_FILE} "*/\n")

foreach (_current_FILE ${COMBINEDHEADERSICAL})
   FILE(READ ${_current_FILE} _contents)
   STRING(REGEX REPLACE "#include *\"ical.*\\.h\"" "" _contents "${_contents}")
   STRING(REGEX REPLACE "#include *\"config.*\\.h\"" "" _contents "${_contents}")
   STRING(REGEX REPLACE "#include *\"pvl\\.h\"" "" _contents "${_contents}" )
   #STRING(REGEX REPLACE " *\$$[Id|Locker]: .+\$$" "" _contents "${_contents}")
   FILE(APPEND ${KDE_FILE_H_FILE} "${_contents}")
endforeach (_current_FILE)

FILE(APPEND ${KDE_FILE_H_FILE} "\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "}\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")

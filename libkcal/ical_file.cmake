# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering.
# It is required to make the combined header ical.h properly.
set(COMBINEDHEADERSICAL
   ${TOPS}/libkcal/libical/src/libical/icalversion.h
   ${TOPS}/libkcal/libical/src/libical/icaltime.h
   ${TOPS}/libkcal/libical/src/libical/icalduration.h
   ${TOPS}/libkcal/libical/src/libical/icalperiod.h
   ${TOPS}/libkcal/libical/src/libical/icalenums.h
   ${TOPS}/libkcal/libical/src/libical/icaltypes.h
   ${TOPS}/libkcal/libical/src/libical/icalrecur.h
   ${TOPS}/libkcal/libical/src/libical/icalattach.h
   ${TOPB}/libkcal/libical/src/libical/icalderivedvalue.h
   ${TOPB}/libkcal/libical/src/libical/icalderivedparameter.h
   ${TOPS}/libkcal/libical/src/libical/icalvalue.h
   ${TOPS}/libkcal/libical/src/libical/icalparameter.h
   ${TOPB}/libkcal/libical/src/libical/icalderivedproperty.h
   ${TOPS}/libkcal/libical/src/libical/icalproperty.h
   ${TOPS}/libkcal/libical/src/libical/pvl.h
   ${TOPS}/libkcal/libical/src/libical/icalarray.h
   ${TOPS}/libkcal/libical/src/libical/icalcomponent.h
   ${TOPS}/libkcal/libical/src/libical/icaltimezone.h
   ${TOPS}/libkcal/libical/src/libical/icalparser.h
   ${TOPS}/libkcal/libical/src/libical/icalmemory.h
   ${TOPS}/libkcal/libical/src/libical/icalerror.h
   ${TOPS}/libkcal/libical/src/libical/icalrestriction.h
   ${TOPS}/libkcal/libical/src/libical/sspm.h
   ${TOPS}/libkcal/libical/src/libical/icalmime.h
   ${TOPS}/libkcal/libical/src/libical/icallangbind.h
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

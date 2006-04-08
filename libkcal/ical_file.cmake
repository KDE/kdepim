set(COMBINEDHEADERSICAL
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalversion.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icaltime.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalduration.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalperiod.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalenums.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icaltypes.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalrecur.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalattach.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalderivedvalue.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalderivedparameter.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalvalue.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalparameter.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalderivedproperty.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalproperty.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/pvl.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalarray.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalcomponent.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icaltimezone.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalparser.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalmemory.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalerror.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalrestriction.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/sspm.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icalmime.h
   ${CMAKE_SOURCE_DIR}/libical/src/libical/icallangbind.h

)

FILE(WRITE ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "extern \"C\" {\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")
FILE(APPEND ${KDE_FILE_H_FILE} "/*\n")
FILE(APPEND ${KDE_FILE_H_FILE} " $Id$\n")

FILE(APPEND ${KDE_FILE_H_FILE} "*/\n")

foreach (_current_FILE ${COMBINEDHEADERSICAL})
    MESSAGE( ${CMAKE_SOURCE_DIR})
	FILE(READ ${_current_FILE} _contents)
    STRING(REGEX REPLACE "#include *\"ical.*\\.h\"" "" _contents "${_contents}" )
    STRING(REGEX REPLACE "#include *\"config.*\\.h\"" "" _contents "${_contents}" )
    STRING(REGEX REPLACE "#include *\"pvl\\.h\"" "" _contents "${_contents}" )
    #STRING(REGEX REPLACE " *\$$[Id|Locker]: .+\$$" "" _contents "${_contents}" )
	#STRING(REGEX REPLACE " *\$Id *: .+\$$" ""_contents "${_contents}" )
	#STRING(REGEX REPLACE " *\$Locker *: .+\$$" ""_contents "${_contents}" )
    FILE(APPEND ${KDE_FILE_H_FILE} "${_contents}")
endforeach (_current_FILE ${KDE_FILE_HEADER})

FILE(APPEND ${KDE_FILE_H_FILE} "\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#ifdef __cplusplus\n")
FILE(APPEND ${KDE_FILE_H_FILE} "}\n")
FILE(APPEND ${KDE_FILE_H_FILE} "#endif\n")

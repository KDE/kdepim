
add_subdirectory( tests ) 

include_directories(
	${CMAKE_SOURCE_DIR}/
	
)


########### next target ###############

set( kholidays_ng_LIB_SRC
astroseasons.cpp
dumpholidaylist.cpp
holidaycalendar.cpp
holiday.cpp
holidayfilter.cpp
kholidays.cpp
lunarphase.cpp
zodiac.cpp
)

kde4_automoc(${kholidays_ng_LIB_SRC})
kode_add_xml_parser( kholidays_ng_LIB_SRC kde-holidays.rng )
kde4_add_library( kholidays_ng SHARED ${kholidays_ng_LIB_SRC} )
target_link_libraries( kholidays_ng ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )
set_target_properties( kholidays_ng PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kholidays_ng  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

install( FILES
   astroseasons.h
   holidaycalendar.h
   holidayfilter.h
   holiday.h
   kholidays.h
   kholidays_version.h
   lunarphase.h
   zodiac.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkholiday_ng 
   )



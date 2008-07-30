set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(libfeatureplancommon_SRCS)

kode_add_xml_parser(libfeatureplancommon_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kde-features.rng)
kde4_add_kcfg_files(libfeatureplancommon_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../prefs.kcfgc )

########### next target ###############

set(dumpfeaturelist_SRCS dumpfeaturelist.cpp ${libfeatureplancommon_SRCS})


kde4_add_executable(dumpfeaturelist TEST ${dumpfeaturelist_SRCS})

target_link_libraries(dumpfeaturelist ${QT_QTXML_LIBRARY} ${KDE4_KDEUI_LIBS})


########### next target ###############
set(benchmarkfeaturelist_SRCS benchmarkfeaturelist.cpp ${libfeatureplancommon_SRCS})


kde4_add_executable(benchmarkfeaturelist TEST ${benchmarkfeaturelist_SRCS})

target_link_libraries(benchmarkfeaturelist ${QT_QTXML_LIBRARY} ${KDE4_KDEUI_LIBS})

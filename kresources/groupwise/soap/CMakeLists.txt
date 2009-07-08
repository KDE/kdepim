set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kresources/groupwise  )

########### next target ###############

set(soapdebug_SRCS soapdebug.cpp )

# todo: more appropriate icon?
#kde4_add_app_icon(soapdebug_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kbugbuster.png")

kde4_add_executable(soapdebug TEST ${soapdebug_SRCS})

target_link_libraries(soapdebug ${KDE4_KDECORE_LIBS} gwsoap ${KDEPIMLIBS_KCAL_LIBS} kdepim)

########### next target ###############

set(gwsoap_SRCS
   contactconverter.cpp
   gwconverter.cpp
   incidenceconverter.cpp
   groupwiseserver.cpp
   gwjobs.cpp
   soapC.cpp
   soapClient.cpp
   stdsoap2.cpp
   )

kde4_add_library(gwsoap SHARED ${gwsoap_SRCS})

target_link_libraries(gwsoap ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${QT_QTCORE_LIBRARY})

set_target_properties(gwsoap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS gwsoap ${INSTALL_TARGETS_DEFAULT_ARGS})

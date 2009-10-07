set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_definitions(${QT_QTDBUS_DEFINITIONS})

########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
)

kde4_add_library(kdepimdbusinterfaces SHARED ${kdepimdbusinterfaces_LIB_SRCS})

target_link_libraries(kdepimdbusinterfaces
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

set_target_properties(kdepimdbusinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepimdbusinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})


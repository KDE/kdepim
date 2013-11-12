include_directories(
)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


set(libnoteshared_SRCS
    attributes/notealarmattribute.cpp
    attributes/notedisplayattribute.cpp
    attributes/notelockattribute.cpp
    noteutils.cpp
    network/notesnetworksender.cpp
    network/notesnetworkreceiver.cpp
    network/notehostdialog.cpp
    editor/noteeditorutils.cpp
)

set( libnoteshared_kcfg_lib_SRCS settings/notesharedglobalconfig.kcfgc )

kde4_add_kcfg_files(libnoteshared_kcfg_lib_SRCS ${libnoteshared_kcfg_lib_SRCS})


kde4_add_library(noteshared ${LIBRARY_TYPE} ${libnoteshared_SRCS} ${libnoteshared_kcfg_lib_SRCS} )

target_link_libraries(noteshared
     ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTNETWORK_LIBRARY} ${KDE4_KDNSSD_LIBS}
)

set_target_properties(noteshared PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS noteshared ${INSTALL_TARGETS_DEFAULT_ARGS})

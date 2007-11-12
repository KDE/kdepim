
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_p2k_PART_SRCS kio_p2kprotocol.cpp p2kimpl.cpp p2kwrapper.cpp )


#kde4_add_dcop_stubs(kio_p2k_PART_SRCS MESSAGE(STATUS "DCOP stuff was remove into kde4, convert it to dbus please. kde4_add_dcop_stubs doen't exist. We keep it to remember to port to dbus")

kde4_add_plugin(kio_p2k ${kio_p2k_PART_SRCS})

target_link_libraries(kio_p2k ${KDE4_KIO_LIBS} p2kmoto kmobiletools )

install(TARGETS kio_p2k DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  p2k.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

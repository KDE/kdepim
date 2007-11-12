
include_directories( ${CMAKE_SOURCE_DIR}/kmobiletools/kmobiletools/libkmobiletools ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_obex2_PART_SRCS kio_obexprotocol.cpp obeximpl.cpp obexwrapper.cpp )


kde4_add_plugin(kio_obex2 ${kio_obex2_PART_SRCS})

target_link_libraries(kio_obex2  ${KDE4_KIO_LIBS} bfb multicobex bluetooth openobex obexftp )

install(TARGETS kio_obex2 DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  obex2.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

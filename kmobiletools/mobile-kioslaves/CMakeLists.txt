
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(P2KIOSLAVE)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(OBEXIOSLAVE)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_mobile_PART_SRCS mobile.cpp mobileimpl.cpp )


kde4_add_plugin(kio_mobile ${kio_mobile_PART_SRCS})

target_link_libraries(kio_mobile ${KDE4_KIO_LIBS} kmobiletools )

install(TARGETS kio_mobile DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  mobile.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  mobile.desktop DESTINATION ${DATA_INSTALL_DIR}/systemview )

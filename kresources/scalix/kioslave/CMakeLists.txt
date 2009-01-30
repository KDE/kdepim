########### next target ###############

set(kio_scalix_SRCS 
    scalix.cpp)

kde4_add_plugin(kio_scalix ${kio_scalix_SRCS})

target_link_libraries(kio_scalix kdepim ${KDE4_KCAL_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kio_scalix  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES scalix.protocol scalixs.protocol  DESTINATION ${SERVICES_INSTALL_DIR})

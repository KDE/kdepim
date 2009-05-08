
include_directories( 
  ${CMAKE_SOURCE_DIR}/kresources/groupwise
  ${CMAKE_SOURCE_DIR}/kresources/groupwise/soap
)


########### next target ###############

set(kio_groupwise_PART_SRCS groupwise.cpp )


kde4_add_plugin(kio_groupwise ${kio_groupwise_PART_SRCS})



target_link_libraries(kio_groupwise  ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KCAL_LIBS} gwsoap kdepim )

install(TARGETS kio_groupwise  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES groupwise.protocol groupwises.protocol  DESTINATION ${SERVICES_INSTALL_DIR})





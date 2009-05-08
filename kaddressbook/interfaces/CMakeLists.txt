
include_directories(${CMAKE_SOURCE_DIR}/kaddressbook)

########### next target ###############

set(kabinterfaces_LIB_SRCS 
   configurewidget.cpp 
   contacteditorwidget.cpp 
   core.cpp 
   extensionwidget.cpp 
   xxport.cpp )


kde4_add_library(kabinterfaces SHARED ${kabinterfaces_LIB_SRCS})

target_link_libraries(kabinterfaces  ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KABC_LIBS})

set_target_properties(kabinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES kaddressbook_contacteditorwidget.desktop kaddressbook_extension.desktop kaddressbook_xxport.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})




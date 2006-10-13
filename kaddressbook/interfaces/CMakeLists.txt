



########### next target ###############

set(kabinterfaces_LIB_SRCS 
   configurewidget.cpp 
   contacteditorwidget.cpp 
   core.cpp 
   extensionwidget.cpp 
   xxport.cpp )

kde4_automoc(kabinterfaces ${kabinterfaces_LIB_SRCS})

kde4_add_library(kabinterfaces SHARED ${kabinterfaces_LIB_SRCS})

target_link_libraries(kabinterfaces  ${KDE4_KIO_LIBS} kabc)

set_target_properties(kabinterfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kabinterfaces  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES configurewidget.h contacteditorwidget.h core.h extensionwidget.h xxport.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbook_contacteditorwidget.desktop kaddressbook_extension.desktop kaddressbook_xxport.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})




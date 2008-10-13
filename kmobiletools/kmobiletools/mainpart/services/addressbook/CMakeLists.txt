
########### next target ###############

set(kmobiletools_addressbook_service_LIB_SRCS 
   addressbook.cpp
   addressbookentryitem.cpp
   addaddresseedialog.cpp
)

kde4_add_library(kmtaddressbook_service SHARED ${kmobiletools_addressbook_service_LIB_SRCS})

target_link_libraries(kmtaddressbook_service kmobiletoolslib ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KABC_LIBS})

set_target_properties(kmtaddressbook_service PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmtaddressbook_service DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES  addressbook_service.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


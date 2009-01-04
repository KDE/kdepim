
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook ${Boost_INCLUDE_DIR} )

configure_file (config-xxport.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xxport.h )

########### next target ###############

set(kaddrbk_csv_xxport_PART_SRCS csv_xxport.cpp csvimportdialog.cpp dateparser.cpp comboboxheaderview.cpp )


kde4_add_plugin(kaddrbk_csv_xxport ${kaddrbk_csv_xxport_PART_SRCS})



target_link_libraries(kaddrbk_csv_xxport kaddressbookprivate kabinterfaces kabcommon ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_csv_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############
if(QGPGME_FOUND)
set(kaddrbk_vcard_xxport_PART_SRCS vcard_xxport.cpp )


kde4_add_plugin(kaddrbk_vcard_xxport ${kaddrbk_vcard_xxport_PART_SRCS})



target_link_libraries(kaddrbk_vcard_xxport kaddressbookprivate kabinterfaces kabcommon kdepim ${KDE4_KABC_LIBS} ${KDE4_KIO_LIBS} ${QGPGME_LIBRARIES} )

install(TARGETS kaddrbk_vcard_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})
endif(QGPGME_FOUND)

########### next target ###############

set(kaddrbk_kde2_xxport_PART_SRCS kde2_xxport.cpp )


kde4_add_plugin(kaddrbk_kde2_xxport ${kaddrbk_kde2_xxport_PART_SRCS})



target_link_libraries(kaddrbk_kde2_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_kde2_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_bookmark_xxport_PART_SRCS bookmark_xxport.cpp )


kde4_add_plugin(kaddrbk_bookmark_xxport ${kaddrbk_bookmark_xxport_PART_SRCS})



target_link_libraries(kaddrbk_bookmark_xxport kaddressbookprivate kabinterfaces kabcommon ${KDE4_KABC_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS kaddrbk_bookmark_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_eudora_xxport_PART_SRCS eudora_xxport.cpp )


kde4_add_plugin(kaddrbk_eudora_xxport ${kaddrbk_eudora_xxport_PART_SRCS})



target_link_libraries(kaddrbk_eudora_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_eudora_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_ldif_xxport_PART_SRCS ldif_xxport.cpp )


kde4_add_plugin(kaddrbk_ldif_xxport ${kaddrbk_ldif_xxport_PART_SRCS})



target_link_libraries(kaddrbk_ldif_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_ldif_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_opera_xxport_PART_SRCS opera_xxport.cpp )


kde4_add_plugin(kaddrbk_opera_xxport ${kaddrbk_opera_xxport_PART_SRCS})



target_link_libraries(kaddrbk_opera_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_opera_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_pab_xxport_PART_SRCS pab_xxport.cpp pab_mapihd.cpp pab_pablib.cpp )


kde4_add_plugin(kaddrbk_pab_xxport ${kaddrbk_pab_xxport_PART_SRCS})



target_link_libraries(kaddrbk_pab_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kaddrbk_pab_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_gmx_xxport_PART_SRCS gmx_xxport.cpp )


kde4_add_plugin(kaddrbk_gmx_xxport ${kaddrbk_gmx_xxport_PART_SRCS})



target_link_libraries(kaddrbk_gmx_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_gmx_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

if(GNOKII_FOUND)
   set(kaddrbk_gnokii_xxport_PART_SRCS gnokii_xxport.cpp )


   kde4_add_plugin(kaddrbk_gnokii_xxport ${kaddrbk_gnokii_xxport_PART_SRCS})

   

   target_link_libraries(kaddrbk_gnokii_xxport kaddressbookprivate kabinterfaces ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} ${GNOKII_LIBRARIES})

   install(TARGETS kaddrbk_gnokii_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})
   install(FILES gnokii_xxport.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)
   install(FILES gnokii_xxportui.rc DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
endif(GNOKII_FOUND)

install( FILES csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop opera_xxport.desktop pab_xxport.desktop gmx_xxport.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)

install( FILES csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc            bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc opera_xxportui.rc pab_xxportui.rc gmx_xxportui.rc DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)


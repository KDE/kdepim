
########### next target ###############

set(kmobiletoolsmainpart_PART_SRCS 
   deviceconfigdialog.cpp 
   devicemanager.cpp 
   addressdetails.cpp 
   calldialogimpl.cpp 
   calldialogimpl.h 
   deviceslist.cpp 
   popupaddressee.cpp
      popupnumber.cpp
         popupsms.cpp

   editaddressee.cpp 
   exportphonebookdlg.cpp 
   importphonebookdlg.cpp 
   devicehome.cpp 
   newsmsdlg.cpp 
   smslistviewitem.cpp 
   smspart.cpp 
   newdevicewizard/firstpage.cpp
   newdevicewizard/lastpage.cpp
   statusbar.cpp
   kmobiletools_mainpart.cpp 
   newdevicewizard/newdevicewizard.cpp )

kde4_automoc(${kmobiletoolsmainpart_PART_SRCS})

kde4_add_ui_files(kmobiletoolsmainpart_PART_SRCS 
   calldialog.ui 
   editaddressee_ui.ui 
   exportphonebook.ui 
   importphonebook.ui 
   mainWidget.ui 
   ui_newsms.ui
   cfg_filesystem.ui 
   cfg_gammu-engine.ui 
   deviceList.ui 
   newdevicewizard/firstpage.ui
   newdevicewizard/lastpage.ui
   genericDeviceOptions.ui 
#    ui_newdevicewizard.ui 
   wizDeviceFirst.ui )

kde4_add_plugin(kmobiletoolsmainpart WITH_PREFIX ${kmobiletoolsmainpart_PART_SRCS})

target_link_libraries(kmobiletoolsmainpart  kmobiletools kabc_file_core ${KDE4_KHTML_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KRESOURCES_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} )

install(TARGETS kmobiletoolsmainpart DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kmobiletools_mainpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  kmobiletools_mainpart.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )
install( FILES  kmobiletools_devicepart.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )
install( FILES  newdevicewizard/kmobilewizard.png operatorsdata progress.gif
kmobiletools.notifyrc bticon.png  		security.png kmobilebtwizard.png DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )

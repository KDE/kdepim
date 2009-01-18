
add_subdirectory(services/addressbook)

########### next target ###############

set(kmobiletoolsmainpart_PART_SRCS
   servicemodel/servicemodel.h
   servicemodel/servicemodel.cpp
   servicemodel/deviceitem.h
   servicemodel/deviceitem.cpp
   servicemodel/serviceitem.h
   servicemodel/serviceitem.cpp
   servicemodel/treeitem.h
   servicemodel/treeitem.cpp
   #deviceconfigdialog.cpp 
   devicemanager.h
   devicemanager.cpp
   jobsignalmapper.h
   jobsignalmapper.cpp
   jobqueueview/jobqueueview.h
   jobqueueview/jobqueueview.cpp
   jobqueueview/jobitem.h
   jobqueueview/jobitem.cpp
   #addressdetails.cpp 
   #calldialogimpl.cpp 
   #calldialogimpl.h 
   #deviceslist.cpp 
   #popupaddressee.cpp
   #popupnumber.cpp
   #popupsms.cpp
   #pickphonenumberdialog.cpp
   #editaddressee.cpp 
   #exportphonebookdlg.cpp 
   #importphonebookdlg.cpp 
   #devicehome.cpp 
   #newsmsdlg.cpp 
   #smslistviewitem.cpp 
   #smspart.cpp
   newdevicewizard/welcomepage.h
   newdevicewizard/welcomepage.cpp
   newdevicewizard/firstpage.h
   newdevicewizard/firstpage.cpp
   newdevicewizard/lastpage.h
   newdevicewizard/lastpage.cpp
   #statusbar.cpp
   kmobiletools_mainpart.h
   kmobiletools_mainpart.cpp 
   newdevicewizard/devicewizard.h
   newdevicewizard/devicewizard.cpp
   errorlogdialog.cpp )


kde4_add_ui_files(kmobiletoolsmainpart_PART_SRCS 
   #calldialog.ui 
   #editaddressee_ui.ui 
   #exportphonebook.ui 
   #importphonebook.ui 
   mainWidget.ui 
   #ui_newsms.ui
   #pickphonenumber.ui
   cfg_filesystem.ui 
   cfg_gammu-engine.ui 
   deviceList.ui 
   newdevicewizard/firstpage.ui
   newdevicewizard/lastpage.ui
   genericDeviceOptions.ui 
#    ui_newdevicewizard.ui 
   wizDeviceFirst.ui )

kde4_add_plugin(kmobiletoolsmainpart ${kmobiletoolsmainpart_PART_SRCS})

target_link_libraries(kmobiletoolsmainpart  kmobiletoolslib kabc_file_core ${KDE4_KHTML_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBRARY} ${KDE4_KRESOURCES_LIBS} ${KDE4_KABC_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY})

install(TARGETS kmobiletoolsmainpart DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kmobiletools_mainpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  kmobiletools_mainpart.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )
install( FILES  kmobiletools_devicepart.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )
install( FILES  newdevicewizard/kmobilewizard.png newdevicewizard/phone_connection.png operatorsdata progress.gif
kmobiletools.notifyrc bticon.png  		security.png kmobilebtwizard.png DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )

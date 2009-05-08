
include_directories(
	${CMAKE_SOURCE_DIR}/kaddressbook/common
	${CMAKE_BINARY_DIR}/kaddressbook/common
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces
	${CMAKE_SOURCE_DIR}/kaddressbook
	${CMAKE_BINARY_DIR}/kaddressbook
	
)


########### next target ###############

set(kaddrbk_cardview_PART_SRCS 
   cardview.cpp 
   configurecardviewdialog.cpp 
   kaddressbookcardview.cpp )


kde4_add_plugin(kaddrbk_cardview ${kaddrbk_cardview_PART_SRCS})



target_link_libraries(kaddrbk_cardview  kaddressbookprivate kabcommon kdepim ${KDEPIMLIBS_KABC_LIBS}  ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kaddrbk_cardview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_iconview_PART_SRCS kaddressbookiconview.cpp )


kde4_add_plugin(kaddrbk_iconview ${kaddrbk_iconview_PART_SRCS})



target_link_libraries(kaddrbk_iconview kaddressbookprivate kabcommon ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kaddrbk_iconview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_tableview_PART_SRCS configuretableviewdialog.cpp contactlistview.cpp kaddressbooktableview.cpp )


kde4_add_plugin(kaddrbk_tableview ${kaddrbk_tableview_PART_SRCS})



target_link_libraries(kaddrbk_tableview kaddressbookprivate kabcommon kimproxy ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kaddrbk_tableview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES cardview.desktop iconview.desktop tableview.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)


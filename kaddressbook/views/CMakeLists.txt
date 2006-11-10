
include_directories(
	${CMAKE_SOURCE_DIR}/kaddressbook/common
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces
	${CMAKE_SOURCE_DIR}/kaddressbook
	${CMAKE_BINARY_DIR}/kaddressbook
	
)


########### next target ###############

set(kaddrbk_cardview_PART_SRCS ${libkabcommon_SRCS} 
   cardview.cpp 
   colorlistbox.cpp 
   configurecardviewdialog.cpp 
   kaddressbookcardview.cpp )

kde4_automoc(${kaddrbk_cardview_PART_SRCS})

kde4_add_plugin(kaddrbk_cardview WITH_PREFIX ${kaddrbk_cardview_PART_SRCS})



target_link_libraries(kaddrbk_cardview  ${KDE4_KDECORE_LIBS} kaddressbook kabc)

install(TARGETS kaddrbk_cardview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_iconview_PART_SRCS kaddressbookiconview.cpp )

kde4_automoc(${kaddrbk_iconview_PART_SRCS})

kde4_add_plugin(kaddrbk_iconview WITH_PREFIX ${kaddrbk_iconview_PART_SRCS})



target_link_libraries(kaddrbk_iconview ${KDE4_KABC_LIBS} kaddressbook ${LIB_KDE3SUPPORT})

install(TARGETS kaddrbk_iconview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_tableview_PART_SRCS configuretableviewdialog.cpp contactlistview.cpp kaddressbooktableview.cpp )

kde4_automoc(${kaddrbk_tableview_PART_SRCS})

kde4_add_plugin(kaddrbk_tableview WITH_PREFIX ${kaddrbk_tableview_PART_SRCS})



target_link_libraries(kaddrbk_tableview  ${KDE4_KABC_LIBS} kaddressbook ${LIB_KDE3SUPPORT})

install(TARGETS kaddrbk_tableview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES cardview.desktop iconview.desktop tableview.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)


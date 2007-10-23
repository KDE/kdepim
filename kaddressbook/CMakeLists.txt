project(kaddressbook)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( common )
add_subdirectory( interfaces ) 
add_subdirectory( printing ) 
if(QGPGME_FOUND)
add_subdirectory( editors ) 
endif(QGPGME_FOUND)
add_subdirectory( views ) 
add_subdirectory( features ) 
add_subdirectory( kcmconfigs ) 
add_subdirectory( xxport ) 
add_subdirectory( pics ) 
add_subdirectory( csv-templates ) 
add_subdirectory( thumbnailcreator ) 

include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces 
	${CMAKE_CURRENT_SOURCE_DIR}/features 
	${CMAKE_CURRENT_SOURCE_DIR}/views 
	${CMAKE_CURRENT_SOURCE_DIR}/printing 
	${CMAKE_CURRENT_SOURCE_DIR}/common 
	${CMAKE_CURRENT_SOURCE_DIR}/editors 
        ${CMAKE_BINARY_DIR}/libkdepim
	${CMAKE_BINARY_DIR}/kaddressbook/common/
	${Boost_INCLUDE_DIR} )




set(libprinter_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/printing/detailledstyle.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/kabentrypainter.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/mikesstyle.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/ringbinderstyle.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/printingwizard.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/printprogress.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/printstyle.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/selectionpage.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/stylepage.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/printing/printsortmode.cpp
)
MACRO_ADD_FILE_DEPENDENCIES(${CMAKE_CURRENT_SOURCE_DIR}/printing/detailledstyle.cpp ${CMAKE_CURRENT_BINARY_DIR}/ui_ds_appearance.h ${CMAKE_CURRENT_BINARY_DIR}/ui_rbs_appearance.h)

kde4_add_ui_files(libprinter_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/ds_appearance.ui
	${CMAKE_CURRENT_SOURCE_DIR}/printing/rbs_appearance.ui
)

set(libkabim_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/editors/imeditorwidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/editors/imaddresswidget.cpp
)

kde4_add_ui_files(libkabim_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/editors/imeditorbase.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/editors/imaddressbase.ui
)


########### next target ###############

set(kaddressbook_bin_SRCS
	main.cpp 
	kaddressbookmain.cpp 
	)

qt4_add_dbus_adaptor(kaddressbook_bin_SRCS org.kde.KAddressbook.Core.xml kaddressbookmain.h KAddressBookMain kaddressbookadaptor )

kde4_add_executable(kaddressbook_bin ${kaddressbook_bin_SRCS})
set_target_properties(kaddressbook_bin PROPERTIES OUTPUT_NAME kaddressbook)

target_link_libraries(kaddressbook_bin  kaddressbook ${KDE4_KHTML_LIBS})

install(TARGETS kaddressbook_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kaddressbookpart_PART_SRCS kaddressbook_part.cpp )

qt4_add_dbus_adaptor(kaddressbookpart_PART_SRCS org.kde.KAddressbook.Core.xml kaddressbook_part.h KAddressbookPart kaddressbookpartadaptor )


kde4_add_plugin(kaddressbookpart WITH_PREFIX ${kaddressbookpart_PART_SRCS})



target_link_libraries(kaddressbookpart  ${KDE4_KIO_LIBS} kaddressbook )

install(TARGETS kaddressbookpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddressbook_LIB_SRCS ${libkabim_SRCS} ${libprinter_SRCS} 
   addresseeconfig.cpp
   addresseeeditordialog.cpp
   addresseeeditorwidget.cpp
   addresseeeditorbase.cpp
   addresseeeditorextension.cpp
   addresseeutil.cpp
   addresseditwidget.cpp
   addviewdialog.cpp
   emaileditwidget.cpp
   extensionmanager.cpp
   filtereditdialog.cpp
   filterselectionwidget.cpp
   geowidget.cpp
   imagewidget.cpp
   incsearchwidget.cpp
   jumpbuttonbar.cpp
   kabcore.cpp
   kablock.cpp
   kaddressbookservice.cpp
   kaddressbookview.cpp
   keywidget.cpp
   ldapsearchdialog.cpp
   nameeditdialog.cpp
   phoneeditwidget.cpp
   secrecywidget.cpp
   soundwidget.cpp
   viewconfigurefieldspage.cpp
   viewconfigurefilterpage.cpp
   viewconfigurewidget.cpp
   undocmds.cpp
   viewmanager.cpp
   xxportmanager.cpp
   xxportselectdialog.cpp
   contacteditorwidgetmanager.cpp
   simpleaddresseeeditor.cpp
   advancedcustomfields.cpp
   customfieldswidget.cpp
   freebusywidget.cpp
   searchmanager.cpp
   imeditwidget.cpp
   kabtools.cpp 
   distributionlistentryview.cpp
   distributionlisteditor.cpp
   distributionlistpicker.cpp
)

qt4_add_dbus_adaptor(kaddressbook_LIB_SRCS
${CMAKE_SOURCE_DIR}/libkdepim/interfaces/org.kde.addressbook.service.xml kaddressbookservice.h KAddressBookService )

QT4_ADD_DBUS_INTERFACE(kaddressbook_LIB_SRCS org.kde.KAddressbook.Core.xml kaddressbookcore_interface )

kde4_add_library(kaddressbook SHARED ${kaddressbook_LIB_SRCS})

target_link_libraries(kaddressbook  ${KDE4_SYNDICATION_LIBS} kdepim ${KDE4_KCAL_LIBS} kabc_file_core ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_PHONON_LIBS} kabinterfaces kabcommon )

set_target_properties(kaddressbook PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kaddressbook  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kaddressbook.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES zone.tab  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbookui.rc kaddressbook_part.rc  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbook_view.desktop dbusaddressbook.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES org.kde.KAddressbook.Core.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES kaddressbook_export.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kaddressbook)
kde4_install_icons( ${ICON_INSTALL_DIR}   )


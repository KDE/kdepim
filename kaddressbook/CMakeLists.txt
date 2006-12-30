project(kaddressbook)

add_subdirectory( common )
add_subdirectory( interfaces ) 
add_subdirectory( printing ) 
if(GPGME_FOUND)
add_subdirectory( editors ) 
endif(GPGME_FOUND)
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
	${CMAKE_SOURCE_DIR}/akregator/src )


set(libkabcommon_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/kabprefs.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/common/locationmap.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/common/filter.cpp
	)

kde4_add_kcfg_files(libkabcommon_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/kabprefs_base.kcfgc )

set(libkabinterfaces_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces/configurewidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces/contacteditorwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces/core.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces/extensionwidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/interfaces/xxport.cpp
)


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

kde4_add_ui3_files(libkabim_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/editors/imeditorbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/editors/imaddressbase.ui
)


########### next target ###############

set(kaddressbook_bin_SRCS
	main.cpp 
	kaddressbookmain.cpp 
	)

#kde4_add_kcfg_files(kaddressbook_bin_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/kabprefs_base.kcfgc )

kde4_automoc(${kaddressbook_bin_SRCS})

qt4_add_dbus_adaptor(kaddressbook_bin_SRCS org.kde.KAddressbook.Core.xml kaddressbookmain.h KAddressBookMain kaddressbookadaptor )

kde4_add_executable(kaddressbook_bin ${kaddressbook_bin_SRCS})
set_target_properties(kaddressbook_bin PROPERTIES OUTPUT_NAME kaddressbook)

target_link_libraries(kaddressbook_bin  kaddressbook ${KDE4_KHTML_LIBS})

install(TARGETS kaddressbook_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kaddressbookpart_PART_SRCS kaddressbook_part.cpp )

qt4_add_dbus_adaptor(kaddressbookpart_PART_SRCS org.kde.KAddressbook.Core.xml kaddressbook_part.h KAddressbookPart kaddressbookpartadaptor )

kde4_automoc(${kaddressbookpart_PART_SRCS})

kde4_add_plugin(kaddressbookpart WITH_PREFIX ${kaddressbookpart_PART_SRCS})



target_link_libraries(kaddressbookpart  ${KDE4_KDECORE_LIBS} kaddressbook )

install(TARGETS kaddressbookpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddressbook_LIB_SRCS ${libkabinterfaces_SRCS} ${libprinter_SRCS} ${libkabim_SRCS}
                          ${libkabcommon_SRCS}
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
)

kde4_automoc(${kaddressbook_LIB_SRCS})
qt4_add_dbus_adaptor(kaddressbook_LIB_SRCS
${CMAKE_SOURCE_DIR}/libkdepim/interfaces/org.kde.addressbook.service.xml kaddressbookservice.h KAddressBookService )

kde4_add_library(kaddressbook SHARED ${kaddressbook_LIB_SRCS})

target_link_libraries(kaddressbook  ${KDE4_SYNDICATION_LIBS} kdepim ${KDE4_KCAL_LIBS} kabc_file_core ${KDE4_KDEPRINT_LIBS} phononcore )

set_target_properties(kaddressbook PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kaddressbook  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kaddressbook.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES zone.tab  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbookui.rc kaddressbook_part.rc  DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbook_view.desktop dbusaddressbook.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES org.kde.KAddressbook.Core.xml DESTINATION ${DBUS_INTERFACES_DIR} )
kde4_install_icons( ${ICON_INSTALL_DIR}   )


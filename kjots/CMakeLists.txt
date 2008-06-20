project(kjots)

add_subdirectory( icons )
include_directories( ${CMAKE_BINARY_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkdepim )

#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   aboutdata.cpp
   main.cpp
   KJotsMain.cpp
   kjotsentry.cpp
   kjotsedit.cpp
   kjotsbookmarks.cpp
   bookshelf.cpp
   kjotscomponent.cpp
   kjotsreplacenextdialog.cpp
   kjotsbrowser.cpp
   kjotslinkdialog.cpp
   flatcollectionproxymodel.cpp
   kjotsbookshelfentryvalidator.cpp
 )


# D-Bus interface
qt4_generate_dbus_interface( kjotscomponent.h org.kde.KJotsComponent.xml OPTIONS -m )
qt4_add_dbus_interfaces(kjots_SRCS
    ${CMAKE_BINARY_DIR}/kjots/org.kde.KJotsComponent.xml)

kde4_add_ui_files(kjots_SRCS confpagemisc.ui )

kde4_add_kcfg_files(kjots_SRCS KJotsSettings.kcfgc )

kde4_add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots ${KDE4_KIO_LIBS} kdepim)

install(TARGETS kjots  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES Kjots.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kjots.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kjotsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)



#########################################################################
# KPART SECTION
#########################################################################

set(kjotspart_PART_SRCS
   kjotspart.cpp
 )

kde4_add_plugin(kjotspart ${kjotspart_PART_SRCS} ${kjots_SRCS} )

target_link_libraries(kjotspart ${KDE4_KPARTS_LIBS} kdepim)

install( FILES kjotspart.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )




project(kjots)

add_subdirectory( icons )

#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
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
 )

kde4_add_ui_files(kjots_SRCS confpagemisc.ui )

kde4_add_kcfg_files(kjots_SRCS KJotsSettings.kcfgc )

kde4_add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots ${KDE4_KIO_LIBS} )

install(TARGETS kjots  DESTINATION ${BIN_INSTALL_DIR} )


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

target_link_libraries(kjotspart ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} )

install( FILES kjotspart.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )




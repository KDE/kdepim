project(kjots)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)

add_subdirectory( icons )

include_directories( ${KDE4_INCLUDES} )

# Make sure libkdepim/pimapplication.h can be found.
# If building kontact plugin:
include_directories( ${PROJECT_BINARY_DIR}/../ ${PROJECT_SOURCE_DIR}/../ )

#########################################################################
# Code common to the kjots application and the kontact plugin
#########################################################################

set (kjots_common_SRCS
     aboutdata.cpp
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
     knowitimporter.cpp
)

kde4_add_kcfg_files(kjots_common_SRCS KJotsSettings.kcfgc )

kde4_add_ui_files(kjots_common_SRCS confpagemisc.ui )

kde4_add_library(kjots_common ${kjots_common_SRCS} )





#########################################################################
# D-Bus interface
#########################################################################

qt4_generate_dbus_interface( kjotscomponent.h org.kde.KJotsComponent.xml OPTIONS -m )
qt4_add_dbus_interfaces(kjots_SRCS
    ${PROJECT_BINARY_DIR}/org.kde.KJotsComponent.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
)

kde4_add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots
  kjots_common
  ${KDE4_KIO_LIBS}
)

# If building kontact plugin:
target_link_libraries(kjots
  kdepim
)

install(TARGETS kjots  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES Kjots.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kjots.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kjotsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)



# If building kontact plugin:
#########################################################################
# KPART SECTION
#########################################################################

# This part is loaded by kontact if kjots is embedded in it.

set(kjotspart_PART_SRCS
   kjotspart.cpp
 )

kde4_add_plugin(kjotspart ${kjotspart_PART_SRCS}
)


target_link_libraries(kjotspart kjots_common ${KDE4_KPARTS_LIBS} kdepim)

install( FILES kjotspart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )

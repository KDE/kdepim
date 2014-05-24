project(kjots)

kde_enable_exceptions()

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


include_directories(
  ${Boost_INCLUDE_DIRS}
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/noteshared
)

set(kjotsLibs KF5::KIOCore kdepim)

add_subdirectory( icons )

#########################################################################
# Code common to the kjots application and the kontact plugin
# Don't make it a static library, this code needs to be compiled
# with -fPIC for the part and without -fPIC for the executable
#########################################################################

set(kjots_config_SRCS kjotsconfigdlg.cpp)

kconfig_add_kcfg_files(kjots_config_SRCS KJotsSettings.kcfgc )

qt5_wrap_ui(kjots_config_SRCS confpagemisc.ui)


set (kjots_common_SRCS
     aboutdata.cpp
     kjotslockjob.cpp
     kjotsedit.cpp
     kjotstreeview.cpp
     kjotsbookmarks.cpp
     kjotsreplacenextdialog.cpp
     kjotsmodel.cpp
     kjotssortproxymodel.cpp
     kjotswidget.cpp
     kjotsbrowser.cpp
     kjotslinkdialog.cpp
     localresourcecreator.cpp
     kjotsbookshelfentryvalidator.cpp
     knowitimporter.cpp ${kjots_config_SRCS}
)

#########################################################################
# D-Bus interface
#########################################################################

qt5_generate_dbus_interface( kjotswidget.h org.kde.KJotsWidget.xml OPTIONS -m )
qt5_add_dbus_interfaces(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KJotsWidget.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
   ${kjots_common_SRCS}
)
kde4_add_app_icon(kjots_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kjots.png")

add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots
  ${kjotsLibs}
  KF5::KCMUtils
  KF5::AkonadiCore
  KF5::Mime
  KF5::AkonadiMime
  Grantlee::TextDocument
  Grantlee::Templates
  KF5::PimTextEdit
  KF5::KontactInterface
  akonadi_next
  noteshared
)


install(TARGETS kjots  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS Kjots.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kjots.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kjotsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/kjots PATTERN *.svn EXCLUDE )

set(kjotspart_PART_SRCS
  kjotspart.cpp
  ${kjots_common_SRCS}
)

add_library(kjotspart MODULE ${kjotspart_PART_SRCS}
)


target_link_libraries(kjotspart
  KF5::KCMUtils
  kdepim
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  Grantlee::Templates
  Grantlee::TextDocument
  KF5::PimTextEdit
  akonadi_next
  noteshared
)

install( FILES kjotspart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )


###########################################################################
set(kcm_kjots_PART_SRCS kcm_kjots.cpp ${kjots_config_SRCS} )
add_library(kcm_kjots MODULE ${kcm_kjots_PART_SRCS})

target_link_libraries(kcm_kjots   KF5::KCMUtils KF5::KDELibs4Support)

install(TARGETS kcm_kjots DESTINATION ${PLUGIN_INSTALL_DIR})


install(FILES
      kjots_config_misc.desktop
      DESTINATION ${SERVICES_INSTALL_DIR})


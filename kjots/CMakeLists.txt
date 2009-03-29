project(kjots)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)
  include(MacroLibrary)
  include(CheckIncludeFiles)

  include_directories( ${KDE4_INCLUDES} )

  set(KJOTS_BUILD_KONTACT_PLUGIN FALSE)
else("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  option(KJOTS_BUILD_KONTACT_PLUGIN "Build kjots with support for kontact" TRUE)
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

set(kjotsLibs ${KDE4_KIO_LIBS})

if(KJOTS_BUILD_KONTACT_PLUGIN)
  add_definitions(-DKJOTSKONTACTPLUGIN)
  # Make sure libkdepim/pimapplication.h can be found.
  include_directories( ${kdepim_BINARY_DIR} ${kdepim_SOURCE_DIR} )
  set(kjotsLibs ${kjotsLibs} kdepim)
  
endif(KJOTS_BUILD_KONTACT_PLUGIN)

add_subdirectory( icons )


#########################################################################
# Code common to the kjots application and the kontact plugin
# Don't make it a static library, this code needs to be compiled
# with -fPIC for the part and without -fPIC for the executable
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
     kjotsconfigdlg.cpp
)

kde4_add_kcfg_files(kjots_common_SRCS KJotsSettings.kcfgc )

kde4_add_ui_files(kjots_common_SRCS confpagemisc.ui) 

#########################################################################
# D-Bus interface
#########################################################################

qt4_generate_dbus_interface( kjotscomponent.h org.kde.KJotsComponent.xml OPTIONS -m )
qt4_add_dbus_interfaces(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KJotsComponent.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
   ${kjots_common_SRCS}
)

kde4_add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots ${kjotsLibs} )

install(TARGETS kjots  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES Kjots.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kjots.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kjotsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)



# If building kontact plugin:
if ( KJOTS_BUILD_KONTACT_PLUGIN  )
  #########################################################################
  # KPART SECTION
  #########################################################################

  # This part is loaded by kontact if kjots is embedded in it.

  set(kjotspart_PART_SRCS
    kjotspart.cpp
    ${kjots_common_SRCS}
  )

  kde4_add_plugin(kjotspart ${kjotspart_PART_SRCS}
  )


  target_link_libraries(kjotspart ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} kdepim)

  install( FILES kjotspart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
  install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

  install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )


  ###########################################################################
  set(kcm_kjots_PART_SRCS kjotsconfigdlg.cpp )

  kde4_add_kcfg_files(kcm_kjots_PART_SRCS KJotsSettings.kcfgc )

  kde4_add_ui_files(kcm_kjots_PART_SRCS confpagemisc.ui )

  kde4_add_plugin(kcm_kjots ${kcm_kjots_PART_SRCS}) 

  target_link_libraries(kcm_kjots  ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})

  install(TARGETS kcm_kjots DESTINATION ${PLUGIN_INSTALL_DIR})


  install(FILES
        kjots_config_misc.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})



endif ( KJOTS_BUILD_KONTACT_PLUGIN )

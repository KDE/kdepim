project(kleopatra)

if ( NOT KLEO_ONLY_UISERVER OR USABLE_ASSUAN_FOUND )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kleopatra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kleopatra.h )

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${QGPGME_INCLUDES}  )
if (USABLE_ASSUAN_FOUND)
   include_directories(${ASSUAN_INCLUDES} ${GPGME_INCLUDES})
endif(USABLE_ASSUAN_FOUND)
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS ${KDE4_ENABLE_EXCEPTIONS} -D_ASSUAN_ONLY_GPG_ERRORS -UQT_NO_STL )

if ( NOT KLEO_ONLY_UISERVER )
set(libconf_SRCS  ${CMAKE_SOURCE_DIR}/kleopatra/conf/configuredialog.cpp )
add_subdirectory( conf ) 
add_subdirectory( kwatchgnupg ) 
endif ( NOT KLEO_ONLY_UISERVER )
add_subdirectory( tests )

if ( NOT KDEPIMLIBS_FOUND )
   add_definitions( -DKLEO_NO_KDEPIMLIBS )
endif ( NOT KDEPIMLIBS_FOUND )

if ( USABLE_ASSUAN_FOUND )
  set(_kleopatra_uiserver_files
    utils/kdpipeiodevice.cpp
    uiserver/kleo-assuan.cpp
    uiserver/uiserver.cpp
    uiserver/assuanserverconnection.cpp
    uiserver/echocommand.cpp
    uiserver/verifycommand.cpp
    uiserver/decryptcommand.cpp
    )
  if ( WIN32 )
    set( _kleopatra_uiserver_extra_libs ${ASSUAN_VANILLA_LIBRARIES} )
  else ( WIN32 )
    set( _kleopatra_uiserver_extra_libs ${ASSUAN_PTHREAD_LIBRARIES} )
  endif( WIN32 )
  
  if ( HAVE_GPG_ERR_SOURCE_KLEO )
    add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_KLEO )
  else( HAVE_GPG_ERR_SOURCE_KLEO )
    add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_1 )
  endif( HAVE_GPG_ERR_SOURCE_KLEO )

else ( USABLE_ASSUAN_FOUND )
  set(_kleopatra_uiserver_files)
endif ( USABLE_ASSUAN_FOUND )

set( _kleopatra_common_SRCS
   utils/formatting.cpp
   ${_kleopatra_uiserver_files}
   models/keylistmodel.cpp
   models/keylistsortfilterproxymodel.cpp
   models/subkeylistmodel.cpp
   models/useridlistmodel.cpp
   aboutdata.cpp 
   main.cpp )

if ( NOT KLEO_ONLY_UISERVER )
  set( _kleopatra_classical_SRCS
    controllers/keylistcontroller.cpp
    commands/command.cpp
    commands/detailscommand.cpp
    ${libconf_SRCS}
    customactions.cpp 
    certmanager.cpp 
    hierarchyanalyser.cpp 
    certificatewizardimpl.cpp 
    certificateinfowidgetimpl.cpp 
    crlview.cpp 
    certlistview.cpp )

  kde4_add_ui_files( _kleopatra_classical_SRCS certificateinfowidget.ui certificatewizard.ui)

else ( NOT KLEO_ONLY_UISERVER )
  set( _kleopatra_classical_SRCS )
  add_definitions( -DKLEO_ONLY_UISERVER )
endif ( NOT KLEO_ONLY_UISERVER )

kde4_add_executable(kleopatra_bin ${_kleopatra_common_SRCS} ${_kleopatra_classical_SRCS} ${_kleopatra_uiserver_SRCS} )
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

if ( KLEO_ONLY_UISERVER )
  set( _kleopatra_extra_libs )
elseif ( NOT KDEPIMLIBS_FOUND )
  set( _kleopatra_extra_libs ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} )
else( KLEO_ONLY_UISERVER )
  set( _kleopatra_extra_libs ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )
endif ( KLEO_ONLY_UISERVER )

target_link_libraries(kleopatra_bin ${_kleopatra_extra_libs} kleo ${QGPGME_LIBRARIES} ${_kleopatra_uiserver_extra_libs} )

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kleopatraui.rc  DESTINATION ${DATA_INSTALL_DIR}/kleopatra)

else ( NOT KLEO_ONLY_UISERVER OR USABLE_ASSUAN_FOUND )
  message( STATUS "Not building requested UiServer-only kleopatra since no usable assuan was found." )
endif ( NOT KLEO_ONLY_UISERVER OR USABLE_ASSUAN_FOUND )

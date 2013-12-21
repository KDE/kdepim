include_directories(
    ${CMAKE_SOURCE_DIR}/pimcommon/
)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libnoteshared_akonadi_SRCS
    akonadi/notesakonaditreemodel.cpp
    akonadi/noteschangerecorder.cpp
)

set(libnoteshared_SRCS
    attributes/notealarmattribute.cpp
    attributes/notedisplayattribute.cpp
    attributes/notelockattribute.cpp
    attributes/showfoldernotesattribute.cpp
    attributes/attributeregistrar.cpp
    noteutils.cpp
    network/notesnetworksender.cpp
    network/notesnetworkreceiver.cpp
    network/notehostdialog.cpp
    editor/noteeditorutils.cpp
    editor/noteeditor.cpp
    config/noteactionconfig.cpp
    config/notenetworkconfig.cpp
    resources/localresourcecreator.cpp
    job/createnewnotejob.cpp
    dialog/selectednotefolderdialog.cpp
    widget/notelistwidget.cpp
    ${libnoteshared_akonadi_SRCS}
)


macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "resources/metatype.h")
  qt4_add_dbus_interface(libnoteshared_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface( ${CMAKE_CURRENT_SOURCE_DIR}/resources/maildirresource.kcfg
                    org.kde.Akonadi.Maildir.Settings MaildirSettings )

add_custom_target(noteshared_settings_xml ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/maildirresource.kcfg
)



set( libnoteshared_kcfg_lib_SRCS settings/notesharedglobalconfig.kcfgc )

kde4_add_kcfg_files(libnoteshared_kcfg_lib_SRCS ${libnoteshared_kcfg_lib_SRCS})


kde4_add_library(noteshared ${LIBRARY_TYPE} ${libnoteshared_SRCS} ${libnoteshared_kcfg_lib_SRCS} )

target_link_libraries(noteshared
     ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTNETWORK_LIBRARY} ${KDE4_KDNSSD_LIBS} pimcommon akonadi_next ${KDEPIMLIBS_KMIME_LIBS}
)

set_target_properties(noteshared PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS noteshared ${INSTALL_TARGETS_DEFAULT_ARGS})

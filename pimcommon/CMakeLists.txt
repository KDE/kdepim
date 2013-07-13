include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_CURRENT_SOURCE_DIR}/util/
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libpimcommon_SRCS
  acl/aclentrydialog.cpp
  acl/aclmanager.cpp
  acl/aclutils.cpp
  acl/collectionaclpage.cpp
  acl/imapaclattribute.cpp
  util/createresource.cpp
  util/pimutil.cpp
  widgets/minimumcombobox.cpp
  translator/translatorutil.cpp
  translator/translatorwidget.cpp
  translator/abstracttranslator.cpp
  #Laurent: disable it we can't use it directly now
  #translator/babelfishtranslator.cpp
  translator/googletranslator.cpp
  widgets/renamefiledialog.cpp
  widgets/simplestringlisteditor.cpp
  templatewidgets/templatelistwidget.cpp
  templatewidgets/templateeditdialog.cpp
  sievehighlighter/sievesyntaxhighlighter.cpp
)

macro(add_resource_iface _kcfgFile _ifaceName _className _fileName)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_fileName} ${_className}
  )
endmacro()

add_resource_iface(util/imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings imapresourcesettings)

kde4_add_library(pimcommon ${LIBRARY_TYPE} ${libpimcommon_SRCS})

target_link_libraries(pimcommon
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KIMAP_LIBS}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
    kdepim
)

set_target_properties(pimcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS pimcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

if (QT_QTDESIGNER_FOUND)
  set(pimcommonwidgets_PART_SRCS)
  kde4_add_widget_files(pimcommonwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/pimcommon.widgets)

  kde4_add_plugin(pimcommonwidgets ${pimcommonwidgets_PART_SRCS})


  target_link_libraries(pimcommonwidgets  ${KDE4_KIO_LIBS} pimcommon)

  install(TARGETS pimcommonwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()



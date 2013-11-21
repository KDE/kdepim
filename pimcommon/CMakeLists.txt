include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_CURRENT_SOURCE_DIR}/util/
  ${CMAKE_CURRENT_SOURCE_DIR}/shorturl/
  ${QJSON_INCLUDE_DIR}
)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

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
  translator/translatordebugdialog.cpp
  #Laurent: disable it we can't use it directly now
  #translator/babelfishtranslator.cpp
  translator/googletranslator.cpp
  widgets/renamefiledialog.cpp
  widgets/simplestringlisteditor.cpp
  widgets/customtextedit.cpp
  widgets/customtreeview.cpp
  widgets/customtoolswidget.cpp
  widgets/configureimmutablewidgetutils.cpp
  texteditor/plaintexteditor/plaintexteditfindbar.cpp
  texteditor/plaintexteditor/plaintexteditor.cpp
  texteditor/plaintexteditor/plaintexteditorwidget.cpp
  texteditor/commonwidget/textfindreplacewidget.cpp
  texteditor/commonwidget/texteditfindbarbase.cpp
  texteditor/richtexteditor/richtexteditor.cpp
  texteditor/richtexteditor/richtexteditfindbar.cpp
  texteditor/richtexteditor/richtexteditorwidget.cpp
  templatewidgets/templatelistwidget.cpp
  templatewidgets/templateeditdialog.cpp
  templatewidgets/templatemanager.cpp
  sievehighlighter/sievesyntaxhighlighter.cpp
  sievehighlighter/sievesyntaxhighlighterutil.cpp
  shorturl/abstractshorturl.cpp
  shorturl/shorturlwidget.cpp
  shorturl/engine/tinyurlshorturl.cpp
  shorturl/engine/googleshorturl.cpp
  shorturl/shorturlconfigurewidget.cpp
  shorturl/shorturlutils.cpp
  shorturl/shorturlconfiguredialog.cpp
  shorturl/engine/migremeshorturl.cpp
  shorturl/engine/triopabshorturl.cpp
  settings/pimcommonsettings.cpp
  autocorrection/autocorrection.cpp
  autocorrection/autocorrectionwidget.cpp
  autocorrection/autocorrectionlistwidget.cpp
  autocorrection/autocorrectiontreewidget.cpp
  autocorrection/autocorrectionlanguage.cpp
  autocorrection/import/importlibreofficeautocorrection.cpp
  autocorrection/import/importkmailautocorrection.cpp
  autocorrection/import/importabstractautocorrection.cpp
  autocorrection/widgets/lineeditwithautocorrection.cpp
  autocorrection/widgets/richtexteditwithautocorrection.cpp
  stockageservice/stockageserviceabstract.cpp
  nepomukdebug/searchdebugnepomukshowdialog.cpp
  nepomukdebug/sparqlsyntaxhighlighter.cpp
  nepomukdebug/akonadiresultlistview.cpp
)

kde4_add_ui_files(libpimcommon_SRCS
     autocorrection/ui/autocorrectionwidget.ui
)

macro(add_resource_iface _kcfgFile _ifaceName _className _fileName)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_fileName} ${_className}
  )
endmacro()

add_resource_iface(util/imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings imapresourcesettings)

kde4_add_kcfg_files(libpimcommon_SRCS
  settings/pimcommonsetting_base.kcfgc
)

kde4_add_library(pimcommon ${LIBRARY_TYPE} ${libpimcommon_SRCS})

target_link_libraries(pimcommon
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KIMAP_LIBS}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
    ${QJSON_LIBRARIES}
    ${KDE4_SOLID_LIBS}
    ${KDE4_KNEWSTUFF3_LIBS}
    kdepim
)

set_target_properties(pimcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS pimcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(translator/tests)
add_subdirectory(texteditor/plaintexteditor/tests)
add_subdirectory(texteditor/richtexteditor/tests)
add_subdirectory(widgets/tests)
add_subdirectory(shorturl/tests)
add_subdirectory(autocorrection/tests)

if (QT_QTDESIGNER_FOUND)
  set(pimcommonwidgets_PART_SRCS)
  kde4_add_widget_files(pimcommonwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/designer/pimcommon.widgets)

  kde4_add_plugin(pimcommonwidgets ${pimcommonwidgets_PART_SRCS})


  target_link_libraries(pimcommonwidgets  ${KDE4_KIO_LIBS} pimcommon)

  install(TARGETS pimcommonwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif ()



include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(libpimcommon_SRCS
  aclentrydialog.cpp
  aclmanager.cpp
  aclutils.cpp
  collectionaclpage.cpp
  createresource.cpp
  imapaclattribute.cpp
  pimutil.cpp
  minimumcombobox.cpp
  translator/translatorutil.cpp
  translator/translatorwidget.cpp
  translator/abstracttranslator.cpp
  #Laurent: disable it we can't use it directly now
  #translator/babelfishtranslator.cpp
  translator/googletranslator.cpp
  renamefiledialog.cpp
)

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(libpimcommon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface(imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings)

kde4_add_library(pimcommon ${LIBRARY_TYPE} ${libpimcommon_SRCS})

target_link_libraries(pimcommon
    ${KDEPIMLIBS_KABC_LIBS}
    ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KIMAP_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
    kdepim
)

set_target_properties(pimcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS pimcommon ${INSTALL_TARGETS_DEFAULT_ARGS})


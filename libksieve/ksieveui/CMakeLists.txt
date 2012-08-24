include_directories(${CMAKE_SOURCE_DIR}/libksieve)

add_subdirectory(tests)

set(ksieveui_LIB_SRCS
  sievelinenumberarea.cpp
  managesievescriptsdialog.cpp
  sievedebugdialog.cpp
  util.cpp
  vacation.cpp
  vacationdialog.cpp
  sievesyntaxhighlighter.cpp
  sievetextedit.cpp
  sieveeditor.cpp
  sievefindbar.cpp
)

kde4_add_kcfg_files(ksieveui_LIB_SRCS settings.kcfgc)

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES
    INCLUDE "metatype.h"
  )
  qt4_add_dbus_interface(ksieveui_LIB_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

add_resource_iface(imapresource.kcfg org.kde.Akonadi.Imap.Settings ImapSettings)

kde4_add_library(ksieveui ${LIBRARY_TYPE} ${ksieveui_LIB_SRCS})

target_link_libraries(ksieveui
  kmanagesieve
  ksieve
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KDECORE_LIBS}
)

set_target_properties(
  ksieveui PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS ksieveui ${INSTALL_TARGETS_DEFAULT_ARGS})

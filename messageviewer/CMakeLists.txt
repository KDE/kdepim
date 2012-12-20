KDE4_NO_ENABLE_FINAL(messageviewer)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

# KCFG files:
# The main messageviewer.kcfg is configured by CMake and put in the build directory.

if(KDEPIM_ENTERPRISE_BUILD)
  set(LEGACY_MANGLE_FROM_TO_HEADERS true)
  set(LEGACY_BODY_INVITES true)
  set(EXCHANGE_COMPATIBLE_INVITATIONS true)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS true)
else(KDEPIM_ENTERPRISE_BUILD)
  set(LEGACY_MANGLE_FROM_TO_HEADERS false)
  set(LEGACY_BODY_INVITES false)
  set(EXCHANGE_COMPATIBLE_INVITATIONS false)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS false)
endif(KDEPIM_ENTERPRISE_BUILD)

configure_file(messageviewer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messageviewer.kcfg)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

add_subdirectory(about)
add_subdirectory(tests)
add_subdirectory(pics)

if(DEBUG_SIGNATURE)
   add_definitions(-DDEBUG_SIGNATURE)
endif(DEBUG_SIGNATURE)

if(KDEPIM_NO_WEBKIT)
  set(_mailwebview_cpp mailwebview_textbrowser.cpp )
  set(_mailwebview_lib )
else()
  set(_mailwebview_cpp mailwebview_webkit.cpp )
  if(NOT WINCE)
    set(_mailwebview_lib ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
    add_subdirectory(a11y)
  endif(NOT WINCE)
endif()

set(libmessageviewer_SRCS
  antispamconfig.cpp
  attachmentdialog.cpp
  attachmentstrategy.cpp
  bodypartformatter.cpp
  bodypartformatterfactory.cpp
  chiasmuskeyselector.cpp
  configurewidget.cpp
  contactdisplaymessagememento.cpp
  csshelper.cpp
  csshelperbase.cpp
  editorwatcher.cpp
  filehtmlwriter.cpp
  findbar/findbarbase.cpp
  findbar/findbarmailwebview.cpp
  findbar/findbarsourceview.cpp
  globalsettings.cpp
  headerstrategy.cpp
  headerstyle.cpp
  htmlquotecolorer.cpp
  htmlstatusbar.cpp
  iconnamecache.cpp
  invitationsettings.cpp
  kleojobexecutor.cpp
  kxface.cpp
  mailsourceviewer.cpp
  markmessagereadhandler.cpp
  mimetreemodel.cpp
  nodehelper.cpp
  objecttreeemptysource.cpp
  objecttreeparser.cpp
  objecttreeparser_p.cpp
  objecttreeviewersource.cpp
  partnodebodypart.cpp
  pluginloaderbase.cpp
  spamheaderanalyzer.cpp
  teehtmlwriter.cpp
  urlhandlermanager.cpp
  util.cpp
  vcardviewer.cpp
  viewer.cpp
  viewer_p.cpp
  webkitparthtmlwriter.cpp
  renamefiledialog.cpp
  translator/translatorutil.cpp
  translator/translatorwidget.cpp
  translator/abstracttranslator.cpp
  translator/babelfishtranslator.cpp
  translator/googletranslator.cpp
  minimumcombobox.cpp
  ${_mailwebview_cpp}
)

kde4_add_kcfg_files(libmessageviewer_SRCS
  globalsettings_base.kcfgc
)

kde4_add_ui_files(libmessageviewer_SRCS
  settings.ui
  invitationsettings.ui
)

kde4_add_library(messageviewer ${LIBRARY_TYPE} ${libmessageviewer_SRCS})

target_link_libraries(messageviewer messagecore kdepim kpgp kleo
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KMBOX_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPRINTUTILS_LIBS}
  ${QGPGME_LIBRARIES}
  ${_mailwebview_lib}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIR}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer.h attachmentstrategy.h headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)

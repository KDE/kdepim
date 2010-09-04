set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

add_subdirectory(tests)
add_subdirectory(pics)

option(MESSAGEVIEWER_NO_WEBKIT "Use QTextBrowser instead of WebKit for the message viewer" FALSE)
if(MESSAGEVIEWER_NO_WEBKIT)
  add_definitions( -DMESSAGEVIEWER_NO_WEBKIT=1 )
  set(_mailwebview_cpp mailwebview_textbrowser.cpp )
  set(_mailwebview_lib )
else(MESSAGEVIEWER_NO_WEBKIT)
  set(_mailwebview_cpp mailwebview_webkit.cpp )
  if(NOT WINCE)
    set(_mailwebview_lib ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
  endif(NOT WINCE)
endif(MESSAGEVIEWER_NO_WEBKIT)

set(libmessageviewer_SRCS
viewer_p.cpp
viewer.cpp
webkitparthtmlwriter.cpp
htmlstatusbar.cpp
headerstyle.cpp
headerstrategy.cpp
attachmentstrategy.cpp
csshelper.cpp
util.cpp
objecttreeparser_p.cpp
objecttreeparser.cpp
kleojobexecutor.cpp
bodypartformatter.cpp
bodypartformatterfactory.cpp
partnodebodypart.cpp
nodehelper.cpp
urlhandlermanager.cpp
vcardviewer.cpp
iconnamecache.cpp
mailsourceviewer.cpp
mimetreemodel.cpp
spamheaderanalyzer.cpp
antispamconfig.cpp
globalsettings.cpp
configurewidget.cpp
htmlquotecolorer.cpp
editorwatcher.cpp
attachmentdialog.cpp
objecttreeviewersource.cpp
objecttreeemptysource.cpp
filehtmlwriter.cpp
teehtmlwriter.cpp
chiasmuskeyselector.cpp
${_mailwebview_cpp}
findbar/findbar.cpp
pluginloaderbase.cpp
kxface.cpp
csshelperbase.cpp
contactphotomemento.cpp
invitationsettings.cpp
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
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
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

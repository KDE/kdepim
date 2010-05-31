set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

add_subdirectory(tests)
add_subdirectory(pics)

if(KDEPIM_MOBILE_UI)
  add_subdirectory(qml)
endif(KDEPIM_MOBILE_UI)

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
mailwebview.cpp
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

kde4_add_library(messageviewer SHARED ${libmessageviewer_SRCS})

target_link_libraries(messageviewer messagecore kdepim kpgp kleo
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBRARY}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KTNEF_LIBRARY}
  ${QGPGME_LIBRARIES}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KDEWEBKIT_LIBRARY}
)
include_directories(
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIR}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION 0.1.0 SOVERSION 0)
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer.h attachmentstrategy.h headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)

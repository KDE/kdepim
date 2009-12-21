set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

option(WEBKIT_BUILD "Use QtWebKit instead of KHTML" false)

if(WEBKIT_BUILD)
  set(htmlparthtmlwriter_SRCS webkitparthtmlwriter.cpp)
  set(WEBKIT_BUILD true)
else(WEBKIT_BUILD)
  set(htmlparthtmlwriter_SRCS khtmlparthtmlwriter.cpp)
  set(WEBKIT_BUILD false)
endif(WEBKIT_BUILD)

configure_file(config-webkit.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-webkit.h)

set(libmessageviewer_SRCS

viewer_p.cpp
viewer.cpp
${htmlparthtmlwriter_SRCS}
htmlstatusbar.cpp
headerstyle.cpp
headerstrategy.cpp
kmaddrbook.cpp
stringutil.cpp
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
global.cpp
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
)


kde4_add_kcfg_files(libmessageviewer_SRCS
  globalsettings_base.kcfgc
)

kde4_add_ui_files(libmessageviewer_SRCS
  settings.ui
)

kde4_add_library(messageviewer SHARED ${libmessageviewer_SRCS})

if( WEBKIT_BUILD )
  target_link_libraries(messageviewer messagecore kdepim kpgp kleo
   ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
   ${KDE4_KDEUI_LIBS} ${KDE4_KTNEF_LIBRARY} ${QGPGME_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS}
   ${QT_QTWEBKIT_LIBRARY} ${KDE4_KDEWEBKIT_LIBRARY}
  )
else( WEBKIT_BUILD )
  target_link_libraries(messageviewer messagecore kdepim kpgp kleo
   ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
   ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KTNEF_LIBRARY}
   ${QGPGME_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS}
  )
endif( WEBKIT_BUILD )
include_directories(
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIRS}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION 0.1.0 SOVERSION 0)
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer.h attachmentstrategy.h headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)

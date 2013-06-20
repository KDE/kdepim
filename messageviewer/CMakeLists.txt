KDE4_NO_ENABLE_FINAL(messageviewer)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(${Grantlee_INCLUDE_DIRS})

# KCFG files:
# The main messageviewer.kcfg is configured by CMake and put in the build directory.

if(KDEPIM_ENTERPRISE_BUILD)
  set(LEGACY_MANGLE_FROM_TO_HEADERS true)
  set(LEGACY_BODY_INVITES true)
  set(EXCHANGE_COMPATIBLE_INVITATIONS true)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS true)
else()
  set(LEGACY_MANGLE_FROM_TO_HEADERS false)
  set(LEGACY_BODY_INVITES false)
  set(EXCHANGE_COMPATIBLE_INVITATIONS false)
  set(OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS false)
endif()

configure_file(messageviewer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messageviewer.kcfg)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

add_subdirectory(about)
add_subdirectory(tests)
add_subdirectory(pics)

if(DEBUG_SIGNATURE)
   add_definitions(-DDEBUG_SIGNATURE)
endif()

if(KDEPIM_NO_WEBKIT)
  set(_mailwebview_cpp viewer/mailwebview_textbrowser.cpp )
  set(_mailwebview_lib )
else()
  set(_mailwebview_cpp viewer/mailwebview_webkit.cpp )
  if(NOT WINCE)
    set(_mailwebview_lib ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
    add_subdirectory(a11y)
  endif()
endif()

set(libmessageviewer_SRCS
  widgets/attachmentdialog.cpp
  viewer/attachmentstrategy.cpp
  bodypartformatter.cpp
  viewer/bodypartformatterfactory.cpp
  chiasmuskeyselector.cpp
  widgets/configurewidget.cpp
  header/contactdisplaymessagememento.cpp
  csshelper.cpp
  csshelperbase.cpp
  viewer/editorwatcher.cpp
  htmlwriter/filehtmlwriter.cpp
  findbar/findbarbase.cpp
  findbar/findbarmailwebview.cpp
  findbar/findbarsourceview.cpp
  globalsettings.cpp
  header/headerstrategy.cpp
  header/headerstrategy_p.cpp
  header/headerstyle.cpp
  header/briefheaderstyle.cpp
  header/grantleeheaderstyle.cpp
  header/customheaderstyle.cpp
  header/plainheaderstyle.cpp
  header/mobileheaderstyle.cpp
  header/entrepriseheaderstyle.cpp
  header/headerstyle_util.cpp
  header/fancyheaderstyle.cpp
  header/customheadersettingwidget.cpp
  header/grantleeheaderformatter.cpp
  header/grantleethememanager.cpp
  header/grantleetheme.cpp
  header/grantleeheaderteststyle.cpp
  header/kxface.cpp
  viewer/htmlquotecolorer.cpp
  widgets/htmlstatusbar.cpp
  widgets/vcardviewer.cpp
  utils/iconnamecache.cpp
  widgets/invitationsettings.cpp
  kleojobexecutor.cpp
  viewer/mailsourceviewer.cpp
  markmessagereadhandler.cpp
  viewer/mimetreemodel.cpp
  viewer/nodehelper.cpp
  viewer/attachmenttemporaryfilesdirs.cpp
  viewer/objecttreeemptysource.cpp
  viewer/objecttreeparser.cpp
  viewer/objecttreeparser_p.cpp
  viewer/objecttreeviewersource.cpp
  viewer/partnodebodypart.cpp
  viewer/pluginloaderbase.cpp
  htmlwriter/teehtmlwriter.cpp
  viewer/urlhandlermanager.cpp
  utils/util.cpp
  viewer/viewer.cpp
  viewer/viewer_p.cpp
  htmlwriter/webkitparthtmlwriter.cpp
  viewer/messagedisplayformatattribute.cpp
  scamdetection/scamdetectionwarningwidget.cpp
  scamdetection/scamdetection.cpp
  scamdetection/scamdetectiondetailsdialog.cpp
  scamdetection/scamattribute.cpp
  antispam/spamheaderanalyzer.cpp
  antispam/antispamconfig.cpp
  ${_mailwebview_cpp}
)

kde4_add_kcfg_files(libmessageviewer_SRCS
  globalsettings_base.kcfgc
)

kde4_add_ui_files(libmessageviewer_SRCS
  ui/settings.ui
  ui/invitationsettings.ui
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
  ${Grantlee_CORE_LIBRARIES} 
  ${KDE4_KNEWSTUFF3_LIBS}
  pimcommon
)

include_directories(
    ${CMAKE_SOURCE_DIR}/messagecore
    ${CMAKE_SOURCE_DIR}/pimcommon
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIR}
    ${GPGME_INCLUDES}
)

set_target_properties( messageviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS messageviewer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES viewer/viewer.h viewer/attachmentstrategy.h header/headerstrategy.h messageviewer_export.h DESTINATION  ${INCLUDE_INSTALL_DIR}/messageviewer COMPONENT devel)


install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/messageviewer )

install(FILES header/data/messageviewer_header_themes.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

add_subdirectory(grantleefilters)

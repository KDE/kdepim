

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_definitions(-DTRANSLATION_DOMAIN=\"libmessageviewer\")

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

configure_file(settings/messageviewer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messageviewer.kcfg)

include(CheckIncludeFiles)
check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
configure_file(config-messageviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messageviewer.h)

# target_include_directories does not handle empty include paths
include_directories(${GPGME_INCLUDES})

if(BUILD_TESTING)
   add_subdirectory(scamdetection/autotests)
   add_subdirectory(findbar/autotests)
   add_subdirectory(scamdetection/tests)
   add_subdirectory(adblock/tests/)
   add_subdirectory(gravatar/autotests/)
   add_subdirectory(header/tests)
   add_subdirectory(header/autotests)
endif()
add_subdirectory(pics)
add_subdirectory(grantleefilters)

if(DEBUG_SIGNATURE)
   add_definitions(-DDEBUG_SIGNATURE)
endif()

if(KDEPIM_NO_WEBKIT)
  set(_mailwebview_cpp viewer/mailwebview_textbrowser.cpp )
  set(_mailwebview_lib )
else()
  set(_mailwebview_cpp viewer/mailwebview_webkit.cpp 
                       adblock/networkaccessmanager.cpp 
                       adblock/webpage.cpp
                       adblock/adblockelementhiding.cpp
                       adblock/adblockhostmatcher.cpp
                       adblock/adblockmanager.cpp
                       adblock/adblockrule.cpp
                       adblock/adblockrulefallbackimpl.cpp
                       adblock/adblockrulenullimpl.cpp
                       adblock/adblockruletextmatchimpl.cpp
                       adblock/adblocksettingwidget.cpp
                       adblock/adblockutil.cpp
                       adblock/adblockaddsubscriptiondialog.cpp
                       adblock/adblocklistwidget.cpp
                       adblock/adblocksyntaxhighlighter.cpp
                       adblock/adblockshowlistdialog.cpp
                       adblock/adblockblockableitemsdialog.cpp
                       adblock/adblockcreatefilterdialog.cpp
                       adblock/adblockblockableitemswidget.cpp
                       )
  set(_mailwebview_lib KF5::WebKit Qt5::WebKitWidgets)
  #reactivate
  #add_subdirectory(a11y)
endif()

set(libmessageviewer_viewer_SRCS
  viewer/attachmentstrategy.cpp
  viewer/bodypartformatter.cpp
  viewer/bodypartformatterfactory.cpp
  viewer/chiasmuskeyselector.cpp
  viewer/cryptohelper.cpp
  viewer/csshelper.cpp
  viewer/csshelperbase.cpp
  viewer/editorwatcher.cpp
  viewer/htmlquotecolorer.cpp
  viewer/mimeparttree/mimetreemodel.cpp
  viewer/nodehelper.cpp
  viewer/objecttreeemptysource.cpp
  viewer/objecttreeparser.cpp
  viewer/objecttreeviewersource.cpp
  viewer/partnodebodypart.cpp
  viewer/pluginloaderbase.cpp
  viewer/viewer.cpp
  viewer/viewer_p.cpp
  viewer/messagedisplayformatattribute.cpp
  viewer/urlhandlermanager.cpp
  viewer/mimeparttree/mimeparttreeview.cpp
  viewer/converthtmltoplaintext.cpp
  viewer/memento/cryptobodypartmemento.cpp
  viewer/memento/decryptverifybodypartmemento.cpp
  viewer/memento/verifydetachedbodypartmemento.cpp
  viewer/memento/verifyopaquebodypartmemento.cpp
)

set(libmessageviewer_widgets_SRCS
  widgets/customheadersettingdialog.cpp
  widgets/attachmentdialog.cpp
  widgets/configurewidget.cpp
  widgets/printingsettings.cpp
  widgets/htmlstatusbar.cpp
  widgets/vcardviewer.cpp
  widgets/invitationsettings.cpp
  widgets/todoedit.cpp
  widgets/eventedit.cpp
  widgets/noteedit.cpp
  widgets/openattachmentfolderwidget.cpp
  widgets/eventdatetimewidget.cpp
  widgets/mailsourceviewer.cpp
  widgets/mailsourceviewtextbrowserwidget.cpp
)

set(libmessageviewer_gravatar_SRCS
  gravatar/gravatarconfigwidget.cpp
  gravatar/gravatarconfiguresettingsdialog.cpp
)

set(libmessageviewer_header_SRCS
  header/contactdisplaymessagememento.cpp
  header/headerstrategy.cpp
  header/headerstrategy_p.cpp
  header/headerstyle.cpp
  header/briefheaderstyle.cpp
  header/grantleeheaderstyle.cpp
  header/customheaderstyle.cpp
  header/plainheaderstyle.cpp
  header/enterpriseheaderstyle.cpp
  header/headerstyle_util.cpp
  header/fancyheaderstyle.cpp
  header/customheadersettingwidget.cpp
  header/grantleeheaderformatter.cpp
  header/grantleeheaderteststyle.cpp
  header/kxface.cpp
  header/headerstyleplugin.cpp
  header/headerstylepluginmanager.cpp
  header/headerstyleinterface.cpp
  header/headerstylemenumanager.cpp
)

set(libmessageviewer_scamdetection_SRCS
  scamdetection/scamdetectionwarningwidget.cpp
  scamdetection/scamdetection.cpp
  scamdetection/scamdetectiondetailsdialog.cpp
  scamdetection/scamattribute.cpp
  scamdetection/scamcheckshorturl.cpp
)

set(libmessageviewer_findbar_SRCS
  findbar/findbarbase.cpp
  findbar/findbarmailwebview.cpp
  findbar/findbarsourceview.cpp
)

set(libmessageviewer_utils_SRCS
  utils/iconnamecache.cpp
  utils/markmessagereadhandler.cpp
  utils/messageviewerutil.cpp
)

set(libmessageviewer_htmlwriter_SRCS
  htmlwriter/filehtmlwriter.cpp
  htmlwriter/teehtmlwriter.cpp
  htmlwriter/webkitparthtmlwriter.cpp
)

set(libmessageviewer_antispam_SRCS
  antispam/spamheaderanalyzer.cpp
  antispam/antispamconfig.cpp
)

set(libmessageviewer_job_SRCS
  job/createtodojob.cpp
  job/createeventjob.cpp
  job/createnotejob.cpp
  job/attachmentencryptwithchiasmusjob.cpp
  job/attachmenteditjob.cpp
  job/modifymessagedisplayformatjob.cpp
  job/kleojobexecutor.cpp
)

set(libmessageviewer_SRCS
  ${libmessageviewer_viewer_SRCS}
  ${libmessageviewer_widgets_SRCS}
  ${libmessageviewer_header_SRCS}
  ${libmessageviewer_scamdetection_SRCS}
  ${libmessageviewer_findbar_SRCS}
  ${libmessageviewer_utils_SRCS}
  ${libmessageviewer_htmlwriter_SRCS}
  ${libmessageviewer_antispam_SRCS}
  ${libmessageviewer_job_SRCS}
  settings/messageviewersettings.cpp
  ${_mailwebview_cpp}
  ${libmessageviewer_gravatar_SRCS}
)

ecm_qt_declare_logging_category(libmessageviewer_SRCS HEADER messageviewer_debug.h IDENTIFIER MESSAGEVIEWER_LOG CATEGORY_NAME log_messageviewer)

kconfig_add_kcfg_files(libmessageviewer_SRCS
  settings/globalsettings_messageviewer.kcfgc
)

ki18n_wrap_ui(libmessageviewer_SRCS
  ui/settings.ui
  ui/invitationsettings.ui
  ui/printingsettings.ui
  adblock/ui/settings_adblock.ui
  adblock/ui/adblockcreatefilterwidget.ui
)


add_library(KF5MessageViewer ${libmessageviewer_SRCS})
generate_export_header(KF5MessageViewer BASE_NAME messageviewer)
add_library(KF5::MessageViewer ALIAS KF5MessageViewer)

target_include_directories(KF5MessageViewer INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/MessageViewer/;${KDE_INSTALL_INCLUDEDIR_KF5}/messageviewer>")
target_include_directories(KF5MessageViewer PUBLIC "$<BUILD_INTERFACE:${MessageViewer_SOURCE_DIR}/src;${MessageViewer_BINARY_DIR}/src;>")

target_link_libraries(KF5MessageViewer
PUBLIC
  KF5::MessageCore
  pimcommon
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Contacts
PRIVATE
  KF5::ItemViews
  ${_mailwebview_lib}
  kdepim
  kleo
  KF5::GrantleeTheme
  KF5::KaddressbookGrantlee
  incidenceeditorsng
  Grantlee5::TextDocument
  Grantlee5::Templates
  Qt5::WebKitWidgets
  KF5::MailTransport
  KF5::Mime
  KF5::Mbox
  KF5::KDELibs4Support
  KF5::AkonadiNotes
)
if (WIN32)
  target_link_libraries(KF5MessageViewer PUBLIC KF5::Gpgmepp)
else()
  target_link_libraries(KF5MessageViewer PUBLIC KF5::Gpgmepp-pthread)
endif()

if(NOT KDEPIM_NO_WEBKIT)
  target_include_directories(KF5MessageViewer PUBLIC $<TARGET_PROPERTY:KF5::WebKit,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

set_target_properties(KF5MessageViewer PROPERTIES
    VERSION ${MESSAGEVIEWER_VERSION_STRING}
    SOVERSION ${MESSAGEVIEWER_SOVERSION}
    EXPORT_NAME MessageViewer
)

install(TARGETS
    KF5MessageViewer
    EXPORT KF5MessageViewerTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(MessageViewer_CamelCase_HEADERS
  HEADER_NAMES
  viewer/Viewer
  viewer/CSSHelper
  viewer/ObjectTreeParser
  viewer/AttachmentStrategy
  viewer/MessageDisplayFormatAttribute
  viewer/NodeHelper
  viewer/EditorWatcher
  widgets/InvitationSettings
  widgets/PrintingSettings
  widgets/ConfigureWidget
  REQUIRED_HEADERS MessageViewer_HEADERS
  PREFIX MessageViewer
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${MessageViewer_SOURCE_DIR})
install(FILES
    ${MessageViewer_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/MessageViewer
    COMPONENT Devel
)

install(FILES
    ${MessageViewer_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/messageviwer_export.h
    ${MessageViewer_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/messageviewer
    COMPONENT Devel
)
endif()


install(DIRECTORY themes DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer )

install(FILES header/data/messageviewer_header_themes.knsrc DESTINATION ${KDE_INSTALL_CONFDIR} )

install(FILES notify/messageviewer.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )
install(FILES scamdetection/data/longurlServices.json DESTINATION ${KDE_INSTALL_DATADIR}/messageviewer )

add_subdirectory(header)

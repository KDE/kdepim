set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(libmailreader_SRCS

kmreaderwin.cpp
khtmlparthtmlwriter.cpp
htmlstatusbar.cpp
headerstyle.cpp
headerstrategy.cpp
kmaddrbook.cpp
stringutil.cpp
attachmentstrategy.cpp
kmmsgpartdlg.cpp
isubject.cpp
csshelper.cpp
util.cpp
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
 )


kde4_add_kcfg_files(libmailreader_SRCS
  globalsettings_base.kcfgc
)

kde4_add_ui_files(libmailreader_SRCS
  settings.ui
)

kde4_add_library(mailreader SHARED ${libmailreader_SRCS})


target_link_libraries(mailreader kdepim mimelib kpgp kleo akonadi-kde
 ${KDEPIMLIBS_KMIME_LIBS}
 ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KTNEF_LIBRARY}
 ${QGPGME_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/libkleo
    ${Boost_INCLUDE_DIRS}
    ${GPGME_INCLUDES}
)

install(TARGETS mailreader ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kmreaderwin.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/mailreader COMPONENT devel)

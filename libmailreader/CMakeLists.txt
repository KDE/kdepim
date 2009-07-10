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
kmmimeparttree.cpp
objecttreeparser.cpp
kleojobexecutor.cpp
bodypartformatter.cpp
bodypartformatterfactory.cpp
partnodebodypart.cpp
 )

configure_file(kmail.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmail.kcfg)

kde4_add_library(mailreader SHARED ${libmailreader_SRCS})

target_link_libraries(mailreader kdepim mimelib kpgp kleo akonadi-kde
 ${KDEPIMLIBS_KMIME_LIBS}
 ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KTNEF_LIBRARY}
 ${QGPGME_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS}
)

install(TARGETS mailreader ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kmreaderwin.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/mailreader COMPONENT devel)

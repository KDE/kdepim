add_subdirectory(lib/mimelib)
add_subdirectory(lib/libkpgp)
add_subdirectory(lib/libkdepim)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${KDE4_DATA_DIR}/cmake/modules)

add_subdirectory(lib/libkleo)

set(mailreader_SRCS
   mailreader.cpp
   main.cpp
   mailreaderview.cpp

lib/kmreaderwin.cpp
lib/partNode.cpp
lib/khtmlparthtmlwriter.cpp
lib/htmlstatusbar.cpp
lib/headerstyle.cpp
lib/headerstrategy.cpp
lib/kmaddrbook.cpp
lib/stringutil.cpp
lib/attachmentstrategy.cpp
lib/kmmessage.cpp
lib/kmmsgbase.cpp
lib/kmmsgpart.cpp
lib/kmmsginfo.cpp
lib/isubject.cpp
lib/csshelper.cpp
lib/util.cpp
lib/kmmessagetag.cpp
lib/kmmimeparttree.cpp
lib/objecttreeparser.cpp
lib/kleojobexecutor.cpp
lib/bodypartformatter.cpp
lib/bodypartformatterfactory.cpp
lib/partnodebodypart.cpp
 )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/mimelib)


configure_file(lib/kmail.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/kmail.kcfg)

kde4_add_ui_files(mailreader_SRCS mailreaderview.ui prefs_base.ui)

kde4_add_kcfg_files(mailreader_SRCS settings.kcfgc )

kde4_add_executable(mailreader ${mailreader_SRCS})

target_link_libraries(mailreader kdepim mimelib kpgp  kleo ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS}    ${KDE4_KTNEF_LIBRARY}
 ${QGPGME_LIBRARIES}
)

install(TARGETS mailreader ${INSTALL_TARGETS_DEFAULT_ARGS} )



########### install files ###############

install( FILES mailreader.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES mailreader.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES mailreaderui.rc  DESTINATION  ${DATA_INSTALL_DIR}/mailreader )

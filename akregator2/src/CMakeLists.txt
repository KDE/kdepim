set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( about )
add_subdirectory( icons )

find_package(Akonadi REQUIRED)

include_directories (
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/akregator2/interfaces
    ${CMAKE_BINARY_DIR}/akregator2/interfaces
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${NEPOMUK_CORE_INCLUDE_DIR}
    ${SOPRANO_INCLUDE_DIR}
    )

########### next target ###############

set(akregator2_SRCS main.cpp mainwindow.cpp )

kde4_add_app_icon(akregator2_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/akregator2.png")

kde4_add_executable(akregator2 ${akregator2_SRCS})

target_link_libraries(akregator2 ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} kdepim akregator2private
                      akregator2interfaces )

install(TARGETS akregator2  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(akregator2private_LIB_SRCS
   aboutdata.cpp
   trayicon.cpp
)

kde4_add_library(akregator2private SHARED ${akregator2private_LIB_SRCS})

target_link_libraries(akregator2private kdepim  ${KDE4_KHTML_LIBS} akregator2interfaces)
target_link_libraries(akregator2private LINK_INTERFACE_LIBRARIES ${KDE4_KHTML_LIBS} )

set_target_properties(akregator2private
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS akregator2private ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set( akregator2_utils_SRCS
     utils/filtercolumnsproxymodel.cpp
)

set(akregator2part_PART_SRCS
   ${akregator2_utils_SRCS}
   kernel.cpp
   abstractselectioncontroller.cpp
   articlematcher.cpp #PENDING(frank) remove/port/move to krss
   command_p.cpp
   feediconmanager.cpp
   feedlistview.cpp
   feedpropertiesdialog.cpp
   pluginmanager.cpp
   selectioncontroller.cpp
   utils.cpp
   searchbar.cpp
   browserrun.cpp
   articlelistview.cpp
   actionmanager.cpp
   actions.cpp
   actionmanagerimpl.cpp
   createfeedcommand.cpp
   createfoldercommand.cpp
   editfeedcommand.cpp
   frame.cpp
   framemanager.cpp
   browserframe.cpp
   browserframe_p.cpp
   openurlrequest.cpp
   articleviewer.cpp
   articleformatter.cpp
   addfeeddialog.cpp
   exportfeedlistcommand.cpp
   importfeedlistcommand.cpp
   tabwidget.cpp
   modifycommands.cpp
   akregator2_part.cpp
   mainwidget.cpp
   speechclient.cpp
#    searchdescriptionattribute.cpp
#    searchpatterneditor.cpp
#    searchpatternrow.cpp
   searchproxymodel.cpp
#    searchwindow.cpp
)

qt4_add_dbus_interfaces(akregator2part_PART_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)
qt4_add_dbus_adaptor( akregator2part_PART_SRCS org.kde.akregator2.part.xml akregator2_part.h Akregator2::Part )


kde4_add_ui_files(akregator2part_PART_SRCS
   addfeedwidgetbase.ui feedpropertiesdialog.ui
)

kde4_add_plugin(akregator2part ${akregator2part_PART_SRCS})

target_link_libraries(akregator2part
                      ${KDE4_KUTILS_LIBS}
                      ${KDE4_KNOTIFYCONFIG_LIBS}
                      ${KDE4_SOLID_LIBRARY}
                      akregator2interfaces
                      akregator2private
                      kdepim
                      ${KDEPIMLIBS_KPIMUTILS_LIBS}
                      ${KDEPIMLIBS_KRSS_LIBS}
		      ${KDE4_AKONADI_LIBS})

install(TARGETS akregator2part  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregator2.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES akregator2_part.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES articleviewer.rc akregator2_part.rc akregator2.notifyrc  DESTINATION ${DATA_INSTALL_DIR}/akregator2)
install( FILES akregator2_plugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES akregator2_shell.rc  DESTINATION ${DATA_INSTALL_DIR}/akregator2)
install( FILES feed.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES org.kde.akregator2.part.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

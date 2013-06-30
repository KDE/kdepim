project(sendlateragent)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(tests)

set(sendlaterlib_SRCS 
    sendlaterinfo.cpp
    sendlaterdialog.cpp
)

set(sendlateragent_SRCS
   ${sendlaterlib_SRCS}
   sendlateragent.cpp
   sendlaterconfiguredialog.cpp
   sendlatermanager.cpp
   sendlaterjob.cpp
   sendlaterutil.cpp
)
 

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

qt4_add_dbus_adaptor(sendlateragent_SRCS org.freedesktop.Akonadi.SendLaterAgent.xml sendlateragent.h SendLaterAgent)

kde4_add_ui_files(sendlateragent_SRCS ui/sendlaterwidget.ui )

kde4_add_executable(akonadi_sendlater_agent ${sendlateragent_SRCS})

target_link_libraries(akonadi_sendlater_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
)

if (Q_WS_MAC)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.sendlater")
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Send Later")
endif ()

install(TARGETS akonadi_sendlater_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES sendlateragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_sendlater_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_sendlater_agent" )


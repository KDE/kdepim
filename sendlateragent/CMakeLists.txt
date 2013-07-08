project(sendlateragent)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
   ${CMAKE_SOURCE_DIR}/messagecomposer/
   ${CMAKE_SOURCE_DIR}/messagecore/
   ${CMAKE_SOURCE_DIR}/libkleo
)


add_subdirectory(tests)

set(sendlaterlib_SRCS 
    sendlaterinfo.cpp
    sendlaterdialog.cpp
    sendlaterutil.cpp
    sendlatertimedatewidget.cpp
)

kde4_add_kcfg_files(sendlaterlib_SRCS
    sendlateragentsettings.kcfgc 
  )

kde4_add_ui_files(sendlaterlib_SRCS ui/sendlaterwidget.ui)


kde4_add_library( sendlater ${LIBRARY_TYPE} ${sendlaterlib_SRCS} )
target_link_libraries( sendlater ${KDE4_KDEUI_LIBS} )

set_target_properties( sendlater PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS sendlater ${INSTALL_TARGETS_DEFAULT_ARGS} )


set(sendlateragent_SRCS
   sendlateragent.cpp
   sendlaterconfiguredialog.cpp
   sendlatermanager.cpp
   sendlaterjob.cpp
)


 

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

qt4_add_dbus_adaptor(sendlateragent_SRCS org.freedesktop.Akonadi.SendLaterAgent.xml sendlateragent.h SendLaterAgent)

kde4_add_ui_files(sendlateragent_SRCS ui/sendlaterconfigurewidget.ui)

kde4_add_executable(akonadi_sendlater_agent ${sendlateragent_SRCS})

target_link_libraries(akonadi_sendlater_agent
  sendlater
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  messagecomposer
  messagecore
)

if (Q_WS_MAC)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.sendlater")
  set_target_properties(akonadi_sendlater_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Send Later")
endif ()

install(TARGETS akonadi_sendlater_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES sendlateragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_sendlater_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_sendlater_agent" )


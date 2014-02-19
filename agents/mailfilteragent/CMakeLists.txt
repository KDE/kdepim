project(akonadi_mailfilter_agent)

add_subdirectory( kconf_update )

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_SOURCE_DIR}/messagecomposer
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(akonadi_mailfilter_agent_SRCS
  dummykernel.cpp
  filterlogdialog.cpp
  filtermanager.cpp
  mailfilteragent.cpp
  pop3resourceattribute.cpp
)

qt4_add_dbus_adaptor(akonadi_mailfilter_agent_SRCS org.freedesktop.Akonadi.MailFilterAgent.xml mailfilteragent.h MailFilterAgent)

#kde4_add_kcfg_files(akonadi_mailfilter_agent_SRCS settings.kcfgc)
#kde4_add_ui_files(akonadi_mailfilter_agent_SRCS configdialog.ui)

kde4_add_executable(akonadi_mailfilter_agent ${akonadi_mailfilter_agent_SRCS})

if (Q_WS_MAC)
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.mailfilter")
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Email Filter")
endif ()


target_link_libraries(akonadi_mailfilter_agent
  mailcommon
  messagecore
  messagecomposer
  pimcommon
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${QT_QTSCRIPT_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
)

install(TARGETS akonadi_mailfilter_agent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mailfilteragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_mailfilter_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_mailfilter_agent" )

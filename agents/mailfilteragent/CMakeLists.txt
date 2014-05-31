project(akonadi_mailfilter_agent)

add_subdirectory( kconf_update )

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_SOURCE_DIR}/messagecomposer
  ${CMAKE_SOURCE_DIR}/pimcommon
)

kde_enable_exceptions()

set(akonadi_mailfilter_agent_SRCS
  dummykernel.cpp
  filterlogdialog.cpp
  filtermanager.cpp
  mailfilteragent.cpp
  pop3resourceattribute.cpp
)

qt5_add_dbus_adaptor(akonadi_mailfilter_agent_SRCS org.freedesktop.Akonadi.MailFilterAgent.xml mailfilteragent.h MailFilterAgent)

#kconfig_add_kcfg_files(akonadi_mailfilter_agent_SRCS settings.kcfgc)
#ki18n_wrap_ui(akonadi_mailfilter_agent_SRCS configdialog.ui)

add_executable(akonadi_mailfilter_agent ${akonadi_mailfilter_agent_SRCS})

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
  
  
  KF5::AkonadiCore
  KF5::AkonadiMime
  ${QT_QTSCRIPT_LIBRARY}
  
  
  KF5::Mime
  KF5::PimIdentities
)

install(TARGETS akonadi_mailfilter_agent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mailfilteragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_mailfilter_agent.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR} )

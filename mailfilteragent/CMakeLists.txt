project(akonadi_mailfilter_agent)

include_directories(
  ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(akonadi_mailfilter_agent_SRCS
  filtermanager.cpp
  mailfilteragent.cpp
)

#kde4_add_kcfg_files(akonadi_mailfilter_agent_SRCS settings.kcfgc)
#kde4_add_ui_files(akonadi_mailfilter_agent_SRCS configdialog.ui)

kde4_add_executable(akonadi_mailfilter_agent ${akonadi_mailfilter_agent_SRCS})

if (Q_WS_MAC)
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.mailfilter")
  set_target_properties(akonadi_mailfilter_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Email Filter")
endif (Q_WS_MAC)


target_link_libraries(akonadi_mailfilter_agent
  mailcommon
  messagecore
  kdepim
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${QT_QTSCRIPT_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
)

install(TARGETS akonadi_mailfilter_agent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mailfilteragent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

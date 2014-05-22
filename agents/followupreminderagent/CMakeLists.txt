project(followupreminderagent)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
   ${CMAKE_SOURCE_DIR}/messagecomposer/
)

set(followupreminderagent_SRCS
    followupreminderagent.cpp
)

qt4_add_dbus_adaptor(followupreminderagent_SRCS org.freedesktop.Akonadi.FollowUpReminder.xml followupreminderagent.h FollowUpReminderAgent)

#kde4_add_kcfg_files(followupreminderagent_SRCS
#    followupreminderagentsettings.kcfgc
#)

kde4_add_executable(akonadi_followupreminder_agent ${followupreminderagent_SRCS})

target_link_libraries(akonadi_followupreminder_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

if (Q_WS_MAC)
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.akonadi_followupreminder_agent")
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Followup Reminder Agent")
endif ()

install(TARGETS akonadi_followupreminder_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES followupreminder.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

#install(FILES akonadi_followupreminder_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_followupreminder_agent" )


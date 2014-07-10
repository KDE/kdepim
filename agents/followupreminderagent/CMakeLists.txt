project(followupreminderagent)



add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_followupreminder_agent\")

include_directories(
   ${CMAKE_SOURCE_DIR}/messagecomposer/
)

set(followupreminderlib_SRCS 
    followupreminderinfo.cpp
)

add_library( followupreminder ${LIBRARY_TYPE} ${followupreminderlib_SRCS} )
target_link_libraries( followupreminder KF5::AkonadiCore )

set_target_properties( followupreminder PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS followupreminder ${INSTALL_TARGETS_DEFAULT_ARGS} )


set(followupreminderagent_job_SRCS
    jobs/followupreminderjob.cpp
    jobs/followupreminderfinishtaskjob.cpp
)

set(followupreminderagent_SRCS
    followupreminderagent.cpp
    followupremindermanager.cpp
    followupreminderinfodialog.cpp
    followupremindernoanswerdialog.cpp
    followupreminderinfowidget.cpp
    ${followupreminderagent_job_SRCS}
)

qt5_add_dbus_adaptor(followupreminderagent_SRCS org.freedesktop.Akonadi.FollowUpReminder.xml followupreminderagent.h FollowUpReminderAgent)

kconfig_add_kcfg_files(followupreminderagent_SRCS
    followupreminderagentsettings.kcfgc
)

add_executable(akonadi_followupreminder_agent ${followupreminderagent_SRCS})

target_link_libraries(akonadi_followupreminder_agent
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::AkonadiAgentBase
  KF5::Mime
  followupreminder
)

if (Q_WS_MAC)
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.akonadi_followupreminder_agent")
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Followup Reminder Agent")
endif ()

install(TARGETS akonadi_followupreminder_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES followupreminder.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

add_subdirectory(tests)

install(FILES akonadi_followupreminder_agent.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR} )

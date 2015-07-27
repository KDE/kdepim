project(followupreminderagent)



add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_followupreminder_agent\")

set(followupreminderlib_SRCS 
    followupreminderinfo.cpp
    followupreminderutil.cpp
)

kconfig_add_kcfg_files(followupreminderlib_SRCS
    settings/followupreminderagentsettings.kcfgc
)


add_library( followupreminder ${followupreminderlib_SRCS} )
generate_export_header(followupreminder BASE_NAME followupreminder)
target_link_libraries( followupreminder KF5::CoreAddons KF5::ConfigGui Qt5::DBus )

set_target_properties( followupreminder PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )


install( TARGETS followupreminder ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


set(followupreminderagent_job_SRCS
    jobs/followupreminderjob.cpp
    jobs/followupreminderfinishtaskjob.cpp
    jobs/followupremindershowmessagejob.cpp
)

set(followupreminderagent_SRCS
    followupreminderagent_debug.cpp
    followupreminderagent.cpp
    followupremindermanager.cpp
    followupreminderinfodialog.cpp
    followupremindernoanswerdialog.cpp
    followupreminderinfowidget.cpp
    ${followupreminderagent_job_SRCS}
)

qt5_add_dbus_adaptor(followupreminderagent_SRCS org.freedesktop.Akonadi.FollowUpReminder.xml followupreminderagent.h FollowUpReminderAgent)

add_executable(akonadi_followupreminder_agent ${followupreminderagent_SRCS})

target_link_libraries(akonadi_followupreminder_agent
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::AkonadiAgentBase
  KF5::Mime
  KF5::CalendarCore
  KF5::DBusAddons
  followupreminder
  Qt5::Widgets
  KF5::KDELibs4Support
)

if( APPLE )
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.akonadi_followupreminder_agent")
  set_target_properties(akonadi_followupreminder_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Followup Reminder Agent")
endif ()

install(TARGETS akonadi_followupreminder_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES followupreminder.desktop DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")

if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

install(FILES akonadi_followupreminder_agent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR} )

project(korgac)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

# enable exception handling


include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
)

add_subdirectory(pixmaps)
add_subdirectory(tests)

########### next target ###############

set(korgac_SRCS
  koalarmclient.cpp
  koalarmclient_debug.cpp
)

set(korgac_SRCS ${korgac_SRCS} korgacmain.cpp)

set(korgac_SRCS
  ${korgac_SRCS}
  alarmdialog.cpp
  alarmdockwindow.cpp
  mailclient.cpp
)

set(korganizer_xml ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

qt5_add_dbus_interface(korgac_SRCS ${korganizer_xml}
  korganizer_interface
)

qt5_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h
  KOAlarmClient
)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DSERIALIZER_PLUGIN_STATIC)
endif()

kde4_add_app_icon(korgac_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmap/hi*-apps-korganizer.png")

add_executable(korgac ${korgac_SRCS})
if(Q_WS_MAC)
  set_target_properties(korgac PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template
  )
  set_target_properties(korgac PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.pim.korgac"
  )
  set_target_properties(korgac PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "KOrganizer Reminder Client"
  )
endif()

target_link_libraries(korgac
  KF5::AkonadiCalendar
  KF5::AkonadiMime
  calendarsupport
  incidenceeditorsng
  kdepim
  kdepimdbusinterfaces
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::IdentityManagement
  KF5::MailTransport
  KF5::Codecs
  Phonon::phonon4qt5
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(korgac akonadi_serializer_kcalcore)
endif()

install(TARGETS
    korgac ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### install files ###############

install(PROGRAMS
  org.kde.korgac.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
)

install(FILES
  org.kde.korganizer.KOrgac.xml
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)

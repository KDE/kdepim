project(korgac)

option(KORGAC_AKONADI_AGENT "Build Korgac as Akonadi agent instead of a stand-alone process" FALSE)
if(KORGAC_AKONADI_AGENT)
  add_definitions(-DKORGAC_AKONADI_AGENT)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5890)
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

if(KORGAC_AKONADI_AGENT)
  set(korgac_SRCS ${korgac_SRCS} korgacagent.cpp korgacresourcefactory.cpp)
else()
  set(korgac_SRCS ${korgac_SRCS} korgacmain.cpp)
endif()

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

if(KORGAC_AKONADI_AGENT)
  add_library(korgac MODULE ${korgac_SRCS})
else()
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
  KF5::PimIdentities
  KF5::MailTransport
  Phonon::phonon4qt5
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(korgac akonadi_serializer_kcalcore)
endif()

# TODO: move this to FindQt4.cmake
find_library(QT_QMAEMO5_LIBRARY QtMaemo5 HINTS ${QT_LIBRARY_DIR})
if(QT_QMAEMO5_LIBRARY)
  target_link_libraries(korgac ${QT_QMAEMO5_LIBRARY})
endif()

if(KORGAC_AKONADI_AGENT)
  install(TARGETS
    korgac
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  install(FILES
    korgacagent.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/akonadi/agents
  )
else()
  install(TARGETS
    korgac ${INSTALL_TARGETS_DEFAULT_ARGS}
  )
endif()

########### install files ###############

install(PROGRAMS
  korgac.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
)

install(FILES
  org.kde.korganizer.KOrgac.xml
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)

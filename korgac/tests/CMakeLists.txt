set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
)

set(korganizer_xml ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

########### next target ###############

set(testalarmdlg_SRCS testalarmdlg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../alarmdialog.cpp ../mailclient.cpp)

qt5_add_dbus_interface(testalarmdlg_SRCS ${korganizer_xml}
  korganizer_interface
)

add_executable(testalarmdlg ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg
  KF5::AkonadiCalendar
  KF5::AkonadiMime
  calendarsupport
  incidenceeditorsng
  kdepimdbusinterfaces
  KF5::KIOCore
  KF5::Mime
  KF5::PimUtils
  korganizer_core
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::PimIdentities
  KF5::MailTransport
  
  ${KDE4_PHONON_LIBS}
)

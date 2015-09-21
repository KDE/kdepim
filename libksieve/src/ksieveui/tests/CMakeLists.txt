add_executable(sievemanagerdialog sievemanagerdialog.cpp)

target_link_libraries(sievemanagerdialog
  KF5::KSieve
  KF5::KSieveUi
)

add_executable(sievedebugdialog sievedebugdialog.cpp)

target_link_libraries(sievedebugdialog
  KF5::KSieve
  KF5::KSieveUi
)

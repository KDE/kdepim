add_executable(sievemanagerdialog sievemanagerdialog.cpp)

target_link_libraries(sievemanagerdialog
  ksieve
  ksieveui
)

add_executable(sievedebugdialog sievedebugdialog.cpp)

target_link_libraries(sievedebugdialog
  ksieve
  ksieveui
)

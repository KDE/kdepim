include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(storageservice_gui_SRCS storageservice_gui.cpp )
add_executable(storageservice_gui ${storageservice_gui_SRCS})
target_link_libraries(storageservice_gui
  pimcommon
)


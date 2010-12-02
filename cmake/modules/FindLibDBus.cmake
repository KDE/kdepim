# - Try to find the dbus libraries
# Once done this will define
#
# DBUS_FOUND            - system supports dbus
# DBUS_INCLUDE_DIRS     - the dbus include directories
# DBUS_LIBRARY_DIRS     - the directories where the dbus libraries are found
# DBUS_LIBRARIES        - libdbus or libdbus-1 library
# DBUSLAUNCH_EXECUTABLE - the path to the dbus-launch executable for testing

if(WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
  find_path(DBUS_INCLUDE_DIRS dbus/dbus.h
    ${_program_FILES_DIR}/dbus/include
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_INCLUDE_PATH}
  )
  set(DBUS_LIBRARY_DIRS ${_program_FILES_DIR}/dbus/lib)
  find_library(DBUS_LIBRARIES NAMES dbus dbus-1 dbus-1d
    PATHS
      ${DBUS_LIBRARY_DIRS}
      ${CMAKE_INSTALL_PREFIX}/lib
      ${CMAKE_LIBRARY_PATH}
  )

if( DBUS_LIBRARIES AND DBUS_INCLUDE_DIRS )
  set( DBUS_FOUND 1 )
endif( DBUS_LIBRARIES AND DBUS_INCLUDE_DIRS )

else(WIN32)

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(DBUS dbus-1>=1.0)

endif(WIN32)

find_program(DBUSLAUNCH_EXECUTABLE NAMES dbus-launch)

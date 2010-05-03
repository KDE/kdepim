project(mail_grantlee)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
find_package(Grantlee REQUIRED)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_GUI_LIB)
add_definitions(-DQT_NO_KEYWORDS)

include(${Grantlee_USE_FILE})

include_directories(
  ${QT_INCLUDES}
  ${QT_QTGUI_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${Grantlee_INCLUDE_DIR}
)

## Application

configure_file(grantlee_paths.h.cmake ${PROJECT_BINARY_DIR}/grantlee_paths.h)

set(mail_grantlee_SRCS
  main.cpp
  mailwindow.cpp
)

qt4_automoc( mail_grantlee_MOC_SRCS ${mail_grantlee_SRCS} )

set(mail_grantlee_RCSS
  mail_grantlee.qrc
)

qt4_add_resources(
  mail_grantlee_RCS_SRCS
  ${mail_grantlee_RCSS}
)

add_executable(mail_grantlee
  ${mail_grantlee_SRCS}
  ${mail_grantlee_RCS_SRCS}
)

target_link_libraries(
  mail_grantlee
  ${QT_QTCORE_LIBRARIES}
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTWEBKIT_LIBRARIES}
  ${Grantlee_CORE_LIBRARIES}
)

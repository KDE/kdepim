project(konsolekalendar)

add_subdirectory(pics) 

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5860)

########### next target ###############

set(konsolekalendar_SRCS 
   konsolekalendarepoch.cpp 
   konsolekalendardelete.cpp 
   konsolekalendarchange.cpp 
   konsolekalendarvariables.cpp 
   konsolekalendaradd.cpp 
   konsolekalendarexports.cpp 
   konsolekalendar.cpp 
   main.cpp )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/korganizer/printing
  ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
  ${CMAKE_BINARY_DIR}/libkdepimdbusinterfaces
  ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${ZLIB_INCLUDE_DIRS}
)
   

kde4_add_app_icon(konsolekalendar_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-calendar.png")

kde4_add_executable(konsolekalendar NOGUI ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar
                      ${KDE4_KDECORE_LIBS}
                      ${KDEPIMLIBS_KCALUTILS_LIBS}
                      ${KDEPIMLIBS_KCALCORE_LIBS}
                      kdepim
                      calendarsupport
                      akonadi-calendar)

install(TARGETS konsolekalendar  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS konsolekalendar.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})


project(konsolekalendar)

add_subdirectory(pics) 

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
  ${Boost_INCLUDE_DIRS}
  
  ${ZLIB_INCLUDE_DIRS}
)
   

#file(GLOB ICONS_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-calendar.png")
#ecm_add_app_icon(konsolekalendar_SRCS ICONS ${ICONS_SRCS})

add_executable(konsolekalendar ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar
                      
                      KF5::CalendarUtils
                      KF5::CalendarCore
                      kdepim
                      calendarsupport
                      KF5::AkonadiCalendar)

install(TARGETS konsolekalendar  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS konsolekalendar.desktop  DESTINATION ${KDE_INSTALL_APPDIR})


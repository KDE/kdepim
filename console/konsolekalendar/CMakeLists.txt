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

#file(GLOB ICONS_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-calendar.png")
#ecm_add_app_icon(konsolekalendar_SRCS ICONS ${ICONS_SRCS})

ecm_qt_declare_logging_category(konsolekalendar_SRCS HEADER konsolekalendar_debug.h IDENTIFIER KONSOLEKALENDAR_LOG CATEGORY_NAME log_konsolekalendar)

add_executable(konsolekalendar ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar
                      KF5::CalendarUtils
                      KF5::CalendarCore
                      KF5::Libkdepim
                      KF5::CalendarSupport
                      KF5::AkonadiCalendar)

install(TARGETS konsolekalendar  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS konsolekalendar.desktop  DESTINATION ${KDE_INSTALL_APPDIR})


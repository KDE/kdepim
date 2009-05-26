project(konsolekalendar)

add_subdirectory(pixmaps) 

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
   stdcalendar.cpp 
   main.cpp )


kde4_add_app_icon(konsolekalendar_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-calendar.png")

kde4_add_executable(konsolekalendar NOGUI ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kdepim)

install(TARGETS konsolekalendar  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES konsolekalendar.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})


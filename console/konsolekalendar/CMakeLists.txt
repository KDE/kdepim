project(konsolekalendar)

add_subdirectory(pixmaps) 

include_directories(${CMAKE_SOURCE_DIR}/kcal)


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

kde4_automoc(konsolekalendar ${konsolekalendar_SRCS})

kde4_add_executable(konsolekalendar ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim)

install(TARGETS konsolekalendar  DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES konsolekalendar.desktop  DESTINATION ${XDG_APPS_DIR})


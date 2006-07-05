
include_directories(
	${CMAKE_SOURCE_DIR}/akonadiresources/include

)



########### next target ###############

set(akonadicontrol_SRCS 
        main.cpp 
        akonadicontrol.cpp
)

kde4_automoc(${akonadicontrol_SRCS})

kde4_add_executable(akonadicontrol RUN_UNINSTALLED ${akonadicontrol_SRCS})

target_link_libraries(akonadicontrol  ${KDE4_KDECORE_LIBS} )



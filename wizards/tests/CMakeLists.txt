set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(testwizard_SRCS testwizard.cpp ../kconfigpropagator.cpp ../kconfigwizard.cpp )

kde4_add_kcfg_files(testwizard_SRCS myconfig.kcfgc )
kde4_add_executable(testwizard TEST ${testwizard_SRCS})

target_link_libraries(testwizard ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})


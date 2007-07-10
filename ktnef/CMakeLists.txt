project(ktnef)
add_subdirectory( pics ) 

include_directories( ${CMAKE_SOURCE_DIR}/ktnef  )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(ktnef_bin_SRCS 
   ktnefmain.cpp 
   ktnefview.cpp 
   attachpropertydialog.cpp 
   messagepropertydialog.cpp 
   attachpropertydialogbase.cpp
   main.cpp )


kde4_add_ui_files(ktnef_bin_SRCS attachpropertydialogbase.ui )

kde4_add_executable(ktnef_bin ${ktnef_bin_SRCS})
set_target_properties(ktnef_bin PROPERTIES OUTPUT_NAME ktnef)

target_link_libraries(ktnef_bin ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KTNEF_LIBS} )

install(TARGETS ktnef_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES ktnef.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES ktnefui.rc  DESTINATION ${DATA_INSTALL_DIR}/ktnef)


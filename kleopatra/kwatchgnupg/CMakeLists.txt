
include_directories( ${CMAKE_SOURCE_DIR}/libkleo  )


########### next target ###############

set(kwatchgnupg_SRCS 
   kwatchgnupgmainwin.cpp 
   kwatchgnupgconfig.cpp 
   aboutdata.cpp 
   tray.cpp 
   main.cpp )


kde4_add_executable(kwatchgnupg ${kwatchgnupg_SRCS})

target_link_libraries(kwatchgnupg  kleopatra ${KDE4_KUTILS_LIBS} )

install(TARGETS kwatchgnupg  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kwatchgnupg.png kwatchgnupg2.png  DESTINATION ${DATA_INSTALL_DIR}/kwatchgnupg/pics)
install( FILES kwatchgnupgui.rc  DESTINATION ${DATA_INSTALL_DIR}/kwatchgnupg)

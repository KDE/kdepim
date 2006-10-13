



########### next target ###############

set(kabcfrontend_SRCS main.cpp)

kde4_automoc(kabcfrontend ${kabcfrontend_SRCS})

kde4_add_ui_files(kabcfrontend_SRCS mainWindow.ui)

kde4_add_executable(kabcfrontend ${kabcfrontend_SRCS})

target_link_libraries(kabcfrontend  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS kabcfrontend  DESTINATION ${BIN_INSTALL_DIR})


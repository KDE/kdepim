
add_subdirectory( pics )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libkpgp\")


INCLUDE(CheckIncludeFiles)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
configure_file(config-libkpgp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libkpgp.h)

########### next target ###############

set(kpgp_LIB_SRCS
   kpgp_debug.cpp
   kpgpblock.cpp
   kpgpui.cpp
   kpgpkey.cpp
   kpgp.cpp
   kpgpbase.cpp
   kpgpbaseG.cpp
   kpgpbase2.cpp
   kpgpbase5.cpp
   kpgpbase6.cpp )


add_library(kpgp ${kpgp_LIB_SRCS})
generate_export_header(kpgp BASE_NAME libkpgp)

target_link_libraries(kpgp
  PUBLIC
    KF5::WidgetsAddons
  PRIVATE
    KF5::I18n
    Qt5::Widgets
    KF5::ConfigCore
    KF5::Codecs
    KF5::IconThemes
    KF5::WindowSystem
    KF5::CoreAddons
)

set_target_properties(kpgp PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kpgp ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_TESTING)
    add_subdirectory( tests )
endif()

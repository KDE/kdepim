set(kwatchgnupg_SRCS
  ../utils/kdlogtextwidget.cpp
  ../utils/gnupg-helper.cpp
  ../utils/hex.cpp
  ../kleopatra_debug.cpp
  kwatchgnupgmainwin.cpp
  kwatchgnupgconfig.cpp
  aboutdata.cpp
  tray.cpp
  main.cpp
  kwatchgnupg_debug.cpp
)

if(WIN32)
  set(kwatchgnupg_SRCS ${kwatchgnupg_SRCS} ../utils/gnupg-registry.c)
endif()

include_directories(..)

add_executable(kwatchgnupg ${kwatchgnupg_SRCS})


target_link_libraries(kwatchgnupg
  KF5::DBusAddons  
  kleo
)

install(TARGETS kwatchgnupg ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES images/kwatchgnupg.png images/kwatchgnupg2.png DESTINATION ${KDE_INSTALL_DATADIR}/kwatchgnupg/pics)

install(FILES kwatchgnupgui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kwatchgnupg)

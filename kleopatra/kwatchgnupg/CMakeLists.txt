

########### next target ###############

set(kwatchgnupg_SRCS
  ../utils/kdlogtextwidget.cpp
  ../utils/gnupg-helper.cpp
  kwatchgnupgmainwin.cpp
  kwatchgnupgconfig.cpp
  aboutdata.cpp
  tray.cpp
  main.cpp
  )

if ( WIN32 )
  set( kwatchgnupg_SRCS ${kwatchgnupg_SRCS} ../utils/gnupg-registry.c )
endif ( WIN32 )

include_directories( .. )


kde4_add_executable(kwatchgnupg ${kwatchgnupg_SRCS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5153 )

if ( NOT KDE4_KCMUTILS_LIBS )
   set ( KDE4_KCMUTILS_LIBS ${KDE4_KUTILS_LIBS} )
endif ( NOT KDE4_KCMUTILS_LIBS )

target_link_libraries(kwatchgnupg ${KDE4_KDEUI_LIBS} kleo ${KDEPIMLIBS_GPGMEPP_LIBS})

install(TARGETS kwatchgnupg ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES kwatchgnupg.png kwatchgnupg2.png  DESTINATION ${DATA_INSTALL_DIR}/kwatchgnupg/pics)
install( FILES kwatchgnupgui.rc  DESTINATION ${DATA_INSTALL_DIR}/kwatchgnupg)

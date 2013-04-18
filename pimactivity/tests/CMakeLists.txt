project( pimactivitytest )

set( pimactivitytest_SRCS
  main.cpp
  mainwindow.cpp
)

kde4_add_executable(pimactivitytest TEST ${pimactivitytest_SRCS} )
TARGET_LINK_LIBRARIES(pimactivitytest ${KDE4_KDEUI_LIBS} pimactivity  )


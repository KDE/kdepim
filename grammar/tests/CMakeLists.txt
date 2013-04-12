project( grammareditor )

set( grammareditor_SRCS
  main.cpp
)

kde4_add_executable(grammareditor ${grammareditor_SRCS} )
TARGET_LINK_LIBRARIES(grammareditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}  )


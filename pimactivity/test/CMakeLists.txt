PROJECT( pimactivityeditor )


set(pimactivity_SRCS
  main.cpp
  pimactivityeditor.cpp
)

KDE4_ADD_EXECUTABLE(pimactivityeditor ${pimactivity_SRCS} )
TARGET_LINK_LIBRARIES(pimactivityeditor ${KDE4_KDEUI_LIBS} )


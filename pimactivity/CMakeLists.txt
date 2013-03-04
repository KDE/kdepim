PROJECT( pimactivity )

set(pimactivitySources
  main.cpp
)

KDE4_ADD_EXECUTABLE(pimactivity ${pimactivitySources} )
TARGET_LINK_LIBRARIES(pimactivity ${KDE4_KDEUI_LIBS}  )

install(TARGETS pimactivity ${INSTALL_TARGETS_DEFAULT_ARGS})


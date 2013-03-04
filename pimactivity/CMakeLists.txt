PROJECT( pimactivity )

find_package(KActivities 6.1.0 QUIET CONFIG)
set_package_properties(KActivities PROPERTIES DESCRIPTION "KActivities" URL "https://projects.kde.org/projects/kde/kdelibs/kactivities" TYPE REQUIRED PURPOSE "Required for Activities integration in kdepim." )


include_directories(${KACTIVITIES_INCLUDE_DIRS})


set(pimactivitySources
  main.cpp
)

KDE4_ADD_EXECUTABLE(pimactivity ${pimactivitySources} )
TARGET_LINK_LIBRARIES(pimactivity ${KDE4_KDEUI_LIBS} ${KACTIVITIES_LIBRARY} )

install(TARGETS pimactivity ${INSTALL_TARGETS_DEFAULT_ARGS})


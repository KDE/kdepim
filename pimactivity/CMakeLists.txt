PROJECT( pimactivity )

find_package(KActivities 6.2.0 QUIET CONFIG)
set_package_properties(KActivities PROPERTIES DESCRIPTION "KActivities" URL "https://projects.kde.org/projects/kde/kdelibs/kactivities" TYPE REQUIRED PURPOSE "Required for Activities integration in kdepim." )


include_directories(${KACTIVITIES_INCLUDE_DIRS})


set(libpimactivity_SRCS
	widgets/comboboxactivity.cpp
  	widgets/configureidentity.cpp
  	activitymanager.cpp
  	configureactivitywidget.cpp
  	widgets/configurecollections.cpp
  	widgets/configuremailtransport.cpp
  	widgets/activitywarning.cpp
        widgets/abstractconfigurewidget.cpp
)

kde4_add_library(pimactivity ${LIBRARY_TYPE} ${libpimactivity_SRCS})

set_target_properties(pimactivity PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(pimactivity
    ${KDE4_KDEUI_LIBS}
    ${KACTIVITIES_LIBRARY}
    ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
)


install(TARGETS pimactivity ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kcm)


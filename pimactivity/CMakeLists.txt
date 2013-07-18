PROJECT( pimactivity )

find_package(KActivities 6.2.0 QUIET CONFIG)
set_package_properties(KActivities PROPERTIES DESCRIPTION "KActivities" URL "https://projects.kde.org/projects/kde/kdelibs/kactivities" TYPE RECOMMENDED PURPOSE "Required for Activities integration in kdepim." )


if(KActivities_FOUND)

include_directories(${KACTIVITIES_INCLUDE_DIRS} .)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


set(libpimactivity_SRCS
	widgets/comboboxactivity.cpp
  	widgets/configureidentity.cpp
  	activitymanager.cpp
  	configureactivitywidget.cpp
        pimwidgets/mailtransport/transportcomboboxactivity.cpp
        pimwidgets/identity/identitycomboboxactivity.cpp
        pimwidgets/identity/identitymanageractivity.cpp
  	widgets/configurecollections.cpp
  	widgets/configuremailtransport.cpp
  	widgets/activitywarning.cpp
        widgets/abstractconfigurewidget.cpp
)

qt4_add_dbus_adaptor(libpimactivity_SRCS pimwidgets/identity/org.kde.pim.IdentityManagerActivity.xml pimwidgets/identity/identitymanageractivity.h PimActivity::IdentityManagerActivity )

kde4_add_library(pimactivity ${LIBRARY_TYPE} ${libpimactivity_SRCS})

set_target_properties(pimactivity PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(pimactivity
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KACTIVITIES_LIBRARY}
    ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
    ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KPIMUTILS_LIBS} 
    akonadi_next
)


install(TARGETS pimactivity ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kcm)
add_subdirectory(tests)
endif()

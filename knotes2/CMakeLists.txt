project(knotes2)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")


include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/noteshared
  ${CMAKE_BINARY_DIR}/noteshared
  ${CMAKE_SOURCE_DIR}/knotes/
)

########### next target ###############

set(libknotes2private_lib_SRCS
    ../knotes/knoteedit.cpp
    configdialog/knotesimpleconfigdialog.cpp
    configdialog/knoteconfigdialog.cpp
    configdialog/knotedisplayconfigwidget.cpp
    configdialog/knoteeditorconfigwidget.cpp
    configdialog/knotecollectionconfigwidget.cpp
    ../knotes/print/knoteprintselectthemedialog.cpp
    ../knotes/print/knoteprintselectthemecombobox.cpp
    ../knotes/print/knoteprinter.cpp
    alarms/knotesalarm.cpp
    alarms/knotealarmdialog.cpp
    print/knoteprintobject.cpp

)

configure_file( settings/knotesglobalconfig.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/knotesglobalconfig.kcfg )

set( libknotes2private_kcfg_lib_SRCS settings/knotesglobalconfig.kcfgc )

kde4_add_kcfg_files(libknotes2private_kcfg_lib_SRCS ${libknotes2private_kcfg_lib_SRCS})

kde4_add_library(knotes2private ${LIBRARY_TYPE} ${libknotes2private_lib_SRCS} ${libknotes2private_kcfg_lib_SRCS} )

target_link_libraries(knotes2private kdepim ${KDE4_KIO_LIBS} pimcommon ${KDE4_KPRINTUTILS_LIBS} ${KDE4_KCMUTILS_LIBS} ${Grantlee_CORE_LIBRARIES} ${KDE4_KDNSSD_LIBS} ${KDE4_KNEWSTUFF3_LIBS} noteshared akonadi_next)

set_target_properties(knotes2private
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS knotes2private ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)


set(knotes2_SRCS apps/main.cpp apps/application.cpp apps/knotesapp.cpp apps/knotesakonaditray.cpp
                ../knotes/dialog/knoteskeydialog.cpp 
                ../knotes/notes/knotebutton.cpp 
                print/knoteprintselectednotesdialog.cpp
                notes/knote.cpp
                notes/knotedisplaysettings.cpp
                dialog/selectednotefolderdialog.cpp
)


qt4_add_dbus_adaptor(knotes2_SRCS  apps/org.kde.KNotes.xml apps/knotesapp.h KNotesApp )

#kde4_add_app_icon(knotes_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-knotes.png")

kde4_add_executable(knotes2 ${knotes2_SRCS})

target_link_libraries(knotes2 knotes2private pimcommon kdepim ${KDE4_KIO_LIBS}
                             ${KDE4_KCMUTILS_LIBS} ${X11_X11_LIB} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDE4_KPRINTUTILS_LIBS} ${KDE4_KDNSSD_LIBS} noteshared
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  akonadi_next
)

install(TARGETS knotes2  ${INSTALL_TARGETS_DEFAULT_ARGS} )

set(kcm_knotes2_PART_SRCS configdialog/knoteconfigdialog.cpp configdialog/knotedisplayconfigwidget.cpp configdialog/knoteeditorconfigwidget.cpp configdialog/knotecollectionconfigwidget.cpp)

kde4_add_plugin(kcm_knote ${kcm_knotes2_PART_SRCS})

target_link_libraries(kcm_knote knotes2private ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KCMUTILS_LIBS} ${KDE4_KNEWSTUFF3_LIBS} noteshared akonadi_next)

install(TARGETS kcm_knote DESTINATION ${PLUGIN_INSTALL_DIR})



#install( PROGRAMS data/knotes.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
#install( FILES settings/knoteconfig.kcfg ${CMAKE_CURRENT_BINARY_DIR}/knotesglobalconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
#install( FILES data/local.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/knotes)
#install( FILES data/knotesappui.rc data/knotesui.rc  DESTINATION ${DATA_INSTALL_DIR}/knotes)
#install( FILES apps/org.kde.KNotes.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}) 
#install( FILES data/knotes_printing_theme.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )

#install(DIRECTORY print/themes/ DESTINATION ${DATA_INSTALL_DIR}/knotes/print/themes )

install(FILES 
        data/knote_config_collection.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})


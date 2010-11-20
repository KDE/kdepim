include_directories(${CMAKE_SOURCE_DIR}/calendarsupport)

set ( incidencesearcher_SRC
      incidencesearcher.cpp )

kde4_add_executable( incidencesearcher
                     ${incidencesearcher_SRC})

target_link_libraries( incidencesearcher
                       calendarsupport
                       ${KDEPIMLIBS_AKONADI_LIBS}
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDEUI_LIBS}
                       ${KDEPIMLIBS_KCALCORE_LIBS}
                     )

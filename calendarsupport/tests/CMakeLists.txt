set ( incidencesearcher_SRC
      incidencesearcher.cpp )

kde4_add_executable( incidencesearcher
                     ${incidencesearcher_SRC})

target_link_libraries( incidencesearcher
                       calendarsupport
                       akonadi-kde
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDECORE_LIBS} )

include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_BINARY_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/calendarviews/agenda
)

########### next target ###############

set( eventviews_LIB_SRCS
     cellitem.cpp
     eventview.cpp
     agenda.cpp
     agendaitem.cpp
     agendaview.cpp
     timelabels.cpp
     timelabelszone.cpp
     prefs.cpp
     helper.cpp
     alternatelabel.cpp
     timescaleconfigdialog.cpp
     recurrenceactions.cpp
)

kde4_add_kcfg_files(eventviews_LIB_SRCS prefs_base.kcfgc )

kde4_add_ui_files(eventviews_LIB_SRCS timescaleedit_base.ui)

kde4_add_library(eventviews SHARED ${eventviews_LIB_SRCS})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

target_link_libraries(eventviews
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTUITOOLS_LIBRARY}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KMIME_LIBRARY}
                      ${KDEPIMLIBS_KCAL_LIBS}
                      ${KDE4_KUTILS_LIBS}
                      ${ZLIB_LIBRARY}
                      ${KDEPIMLIBS_KPIMIDENTITIES_LIBRARY}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
                      ${KDEPIMLIBS_KHOLIDAYS_LIBS}
                      kdepim
                      akonadi-kcal_next
                      akonadi_next
)

set_target_properties(eventviews PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS eventviews ${INSTALL_TARGETS_DEFAULT_ARGS})

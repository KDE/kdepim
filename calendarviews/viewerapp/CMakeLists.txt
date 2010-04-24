project (calendarviewsapp)

include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../agenda
    ${CMAKE_CURRENT_BINARY_DIR}/../agenda)

set(viewerapp_SRCS
    main.cpp
    mainwindow.cpp
)

kde4_add_ui_files(viewerapp_SRCS
    mainwindow.ui
)

kde4_add_executable(viewerapp ${viewerapp_SRCS})

target_link_libraries(viewerapp
    eventviews
    ${KDE4_KDECORE_LIBS}
    ${KDE4_AKONADI_LIBS}
    ${KDEPIMLIBS_KCAL_LIBS}
    ${QT_QTGUI_LIBS}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

########### install files ###############

#install(TARGETS viewerapp ${INSTALL_TARGETS_DEFAULT_ARGS} )
#install( FILES viewerapp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )

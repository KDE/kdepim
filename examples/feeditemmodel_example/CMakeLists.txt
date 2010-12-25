project(feeditemmodel_example)

set(feeditemmodel_example_SRCS
    main.cpp
)

include_directories( ${krss_SOURCE_DIR} )

kde4_add_executable(example_feeditemmodel ${feeditemmodel_example_SRCS} )

target_link_libraries(example_feeditemmodel
                      krss
                      ${KDE4_KDEUI_LIBRARY}
                      ${KDEPIMLIBS_SYNDICATION_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
)

#install(TARGETS example_feeditemmodel
#        DESTINATION ${BIN_INSTALL_DIR})






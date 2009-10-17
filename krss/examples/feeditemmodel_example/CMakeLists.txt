project(feeditemmodel_example)

set(feeditemmodel_example_SRCS
    main.cpp
)

include_directories( ${CMAKE_SOURCE_DIR}/krss )

kde4_add_executable(example_feeditemmodel ${feeditemmodel_example_SRCS} )

target_link_libraries(example_feeditemmodel
                      krss
                      ${KDEPIMLIBS_SYNDICATION_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
)

#install(TARGETS example_feeditemmodel
#        DESTINATION ${BIN_INSTALL_DIR})






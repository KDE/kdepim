project(kleoadmin)

include_directories( ${CMAKE_SOURCE_DIR}/libkleo )
add_definitions ( ${KDE4_ENABLE_EXCEPTIONS} -D_ASSUAN_ONLY_GPG_ERRORS -UQT_NO_STL )

set(_kleoadmin_SRCS
    main.cpp
    mainwindow.cpp
)

kde4_add_ui_files( _kleoadmin_SRCS mainwidget.ui )

kde4_add_executable(kleoadmin ${_kleoadmin_SRCS} )
target_link_libraries(kleoadmin kleo )

install(TARGETS kleoadmin DESTINATION ${BIN_INSTALL_DIR} )


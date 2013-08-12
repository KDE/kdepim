project(grammar)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories( .)

set(libgrammar_SRCS 
    grammarhighlighter.cpp
    grammarclient.cpp
    backgroundgrammarchecker.cpp
    grammarsettings.cpp
    grammarplugin.cpp
    grammarloader.cpp
    grammarchecker.cpp
    widgets/grammarcomboboxlanguage.cpp
    widgets/grammarconfigurewidget.cpp
)

kde4_add_library(grammar ${LIBRARY_TYPE} ${libgrammar_SRCS})

target_link_libraries(grammar ${KDE4_KDEUI_LIBS})


set_target_properties( grammar PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS grammar ${INSTALL_TARGETS_DEFAULT_ARGS} )

add_subdirectory(plugins)

add_subdirectory(tests)


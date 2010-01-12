project(etm_usage)

find_package(KDE4 REQUIRED)
find_package(Akonadi REQUIRED)
find_package(KdepimLibs REQUIRED)

include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)
include(CheckIncludeFiles)
include(FindKdepimLibs)


include_directories( ${KDE4_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

set(etm_usage_SRCS
    main.cpp
    mainwindow.cpp
    entitytreewidget.cpp
    itemviewerwidget.cpp
    tab1widget.cpp
    tab2widget.cpp
    tab3widget.cpp
    tab4widget.cpp
    tab5widget.cpp
)

kde4_add_executable(etm_usage ${etm_usage_SRCS} )

target_link_libraries(etm_usage messageviewer messagelist kdepim kpgp kleo akonadi-kde
                                        akonadi-kmime akonadi_next ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS}
                                        ${KDE4_KTNEF_LIBRARY} ${QGPGME_LIBRARIES}
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_KABC_LIBS}
                      ${KDEPIMLIBS_KMIME_LIBS}
)

install(TARGETS etm_usage
        DESTINATION ${BIN_INSTALL_DIR})


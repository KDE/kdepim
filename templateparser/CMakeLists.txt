add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(templateparser_LIB_SRCS
  templateparser.cpp
  defaulttemplates.cpp
  templatesutil.cpp
)

if(NOT WINCE)
  set(templateparser_LIB_SRCS
    ${templateparser_LIB_SRCS}
    # All disabled due to QTreeWidget and QToolbox in the UI
    customtemplates.cpp
    customtemplatesmenu.cpp
    templatesconfiguration.cpp
    templatesinsertcommand.cpp
    templatessyntaxhighlighter.cpp
    templatestextedit.cpp
  )
endif()

include_directories(
  ${Boost_INCLUDE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messagecore
  ${CMAKE_SOURCE_DIR}/messageviewer
)

kde4_add_kcfg_files(templateparser_LIB_SRCS templatesconfiguration_kfg.kcfgc)

kde4_add_kcfg_files(templateparser_LIB_SRCS
  customtemplates_kfg.kcfgc
  globalsettings_base.kcfgc
)

if(NOT WINCE)
  kde4_add_ui_files(templateparser_LIB_SRCS
    ui/templatesconfiguration_base.ui
    ui/customtemplates_base.ui
  )
endif()

kde4_add_library(templateparser ${LIBRARY_TYPE} ${templateparser_LIB_SRCS})
target_link_libraries(templateparser
  messageviewer
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  messagecore
  kleo
  kpgp
  ${QGPGME_LIBRARIES}
  ${QT_QTWEBKIT_LIBRARY}
)
set_target_properties(templateparser PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS templateparser ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  customtemplates_kfg.kcfg
  templatesconfiguration_kfg.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

add_subdirectory(tests)

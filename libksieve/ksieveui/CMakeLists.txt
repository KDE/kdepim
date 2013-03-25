include_directories(${CMAKE_SOURCE_DIR}/libksieve
	${CMAKE_SOURCE_DIR}/pimcommon
	${CMAKE_BINARY_DIR}/pimcommon
	)

add_subdirectory(tests)

set(ksieveui_LIB_SRCS
  managesievescriptsdialog.cpp
  sievedebugdialog.cpp
  util.cpp
  vacation/vacation.cpp
  vacation/vacationdialog.cpp
  editor/sievefindbar.cpp
  editor/sievesyntaxhighlighter.cpp
  editor/sievetextedit.cpp
  editor/sieveeditor.cpp
  editor/sievelinenumberarea.cpp
  templates/sievetemplatewidget.cpp
  templates/sievedefaulttemplate.cpp 
  templates/sievetemplateeditdialog.cpp 
)

kde4_add_kcfg_files(ksieveui_LIB_SRCS settings.kcfgc)

kde4_add_library(ksieveui ${LIBRARY_TYPE} ${ksieveui_LIB_SRCS})

target_link_libraries(ksieveui
  kmanagesieve
  ksieve
  pimcommon
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KDECORE_LIBS}
)

set_target_properties(
  ksieveui PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS ksieveui ${INSTALL_TARGETS_DEFAULT_ARGS})

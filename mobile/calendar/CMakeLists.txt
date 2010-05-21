project(korganizer-mobile)

include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(korganizer_mobile_SRCS
  declarativeeditors.cpp
  incidenceview.cpp
  mainview.cpp
  main.cpp
)

kde4_add_executable(korganizer-mobile ${korganizer_mobile_SRCS} )
target_link_libraries(korganizer-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KDEUI_LIBRARY}
  ${KDEPIMLIBS_KCAL_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  akonadi-kcal_next
  incidenceeditorsmobile
  kdepimmobileui
)

install(TARGETS korganizer-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES korganizer-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  MobileIncidenceEditor.qml
  incidence-editor.qml
  korganizer-mobile.qml
  DESTINATION ${DATA_INSTALL_DIR}/korganizer-mobile
)

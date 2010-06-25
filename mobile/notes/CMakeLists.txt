# project(notes-mobile)

include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
  ${PROJECT_SOURCE_DIR}/../lib
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(notes_mobile_SRCS
  notelistproxy.cpp
  mainview.cpp
  main.cpp
)

kde4_add_executable(notes-mobile ${notes_mobile_SRCS} )
target_link_libraries(notes-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBRARY}
  kdepimmobileui
  kdepim
  akonadi_next
)

install(TARGETS notes-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES notes-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES notes-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  notes.qml
  NoteView.qml
  NotesListView.qml
  delete-button.png
  DESTINATION ${DATA_INSTALL_DIR}/notes-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )

# project(notes-mobile)

include_directories(
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(notes_mobile_SRCS
  notelistproxy.cpp
  notesfilterproxymodel.cpp
  mainview.cpp
  main.cpp
)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif (RUNTIME_PLUGINS_STATIC)

kde4_add_executable(notes-mobile ${notes_mobile_SRCS} )
if (WINCE)
  SET_TARGET_PROPERTIES(notes-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif(WINCE)

target_link_libraries(notes-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  kdepimmobileui
  kdepim
  akonadi_next
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(notes-mobile akonadi_serializer_mail)
endif (RUNTIME_PLUGINS_STATIC)

install(TARGETS notes-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES notes-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES notes-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  notes.qml
  NoteActions.qml
  NoteView.qml
  NotesListView.qml
  notes-mobile-background.png
  DESTINATION ${DATA_INSTALL_DIR}/notes-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )

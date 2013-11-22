project(notes-mobile)

include_directories(
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(notes_mobile_SRCS
  notelistproxy.cpp
  notesexporthandler.cpp
  notesfilterproxymodel.cpp
  notesimporthandler.cpp
  mainview.cpp
  main.cpp
  searchwidget.cpp
)

qt4_wrap_ui(notes_mobile_SRCS searchwidget.ui)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif()

kde4_add_app_icon(notes_mobile_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-notes-mobile.png")

kde4_add_executable(notes-mobile ${notes_mobile_SRCS})

target_link_libraries(notes-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KMBOX_LIBS}
  kdepimmobileui
  kdepim
  akonadi_next
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(notes-mobile akonadi_serializer_mail)
endif()

install(TARGETS notes-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS notes-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  BulkActionComponent.qml
  notes.qml
  NoteActions.qml
  NoteView.qml
  NotesListView.qml
  notes-mobile-background.png
  DESTINATION ${DATA_INSTALL_DIR}/notes-mobile
)

kde4_install_icons(${ICON_INSTALL_DIR})

project(etm_usage)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${kdepim_SOURCE_DIR}
  ${kdepim_SOURCE_DIR}/messageviewer
  ${Boost_INCLUDE_DIRS}
)

set(etm_usage_SRCS
  main.cpp
  mailmodel.cpp
  mainwindow.cpp
  entitytreewidget.cpp
  entityfindingwidget.cpp
  mixedtreemodel.cpp
  itemviewerwidget.cpp
  noteviewer.cpp
  desctab.cpp
  tab1widget.cpp
  tab2widget.cpp
  tab2_5widget.cpp
  tab3widget.cpp
  tab4widget.cpp
  tab5widget.cpp
  tab6widget.cpp
  tab7widget.cpp
  unreadmailswidget.cpp
  unreadmailsincollectionswidget.cpp
  entitytreemodelfactory.cpp
  recursiveitemlistwidget.cpp
  collectionmonitoredwidget.cpp
  checkable2.cpp
)

kde4_add_executable(etm_usage ${etm_usage_SRCS})

target_link_libraries(etm_usage
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KHTML_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_QGPGME_LIBSS}
  akonadi_next
  kdepim
  kleo
  kpgp
  messagecore
  messagelist
  messageviewer
)

install(TARGETS etm_usage ${INSTALL_TARGETS_DEFAULT_ARGS})


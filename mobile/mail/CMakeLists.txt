project( kmail-mobile )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(tests)

include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
)

set(kmail_mobile_srcs
  breadcrumbnavigation.cpp
  mainview.cpp
  main.cpp
  messagelistproxy.cpp
)

kde4_add_executable(kmail-mobile ${kmail_mobile_srcs} )

target_link_libraries(kmail-mobile
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBRARY}
    ${KDEPIMLIBS_KMIME_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBRARY}
    akonadi_next
    kdepim
)

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  AttachmentList.qml
  Button.qml
  HeaderView.qml
  kmail-mobile.qml
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)

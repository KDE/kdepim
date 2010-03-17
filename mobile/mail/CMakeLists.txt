project( kmail-mobile )

set(kmail_mobile_srcs
  mainview.cpp
  main.cpp
)

kde4_add_executable(kmail-mobile ${kmail_mobile_srcs} )

target_link_libraries(kmail-mobile
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBRARY}
    kdepim
)

install(TARGETS kmail-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  kmail-mobile.qml
  HeaderView.qml
  DESTINATION ${DATA_INSTALL_DIR}/kmail-mobile
)


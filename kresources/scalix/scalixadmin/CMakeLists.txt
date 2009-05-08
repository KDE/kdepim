project(scalixadmin)

include_directories( 
	${CMAKE_SOURCE_DIR}/libkdepim
)

set(scalixadmin_SRCS
  delegatedialog.cpp
  delegatemanager.cpp
  delegatepage.cpp
  delegateview.cpp
  jobs.cpp
  ldapdialog.cpp
  ldapview.cpp
  main.cpp
  mainwindow.cpp
  otherusermanager.cpp
  otheruserpage.cpp
  otheruserview.cpp
  outofofficepage.cpp
  passwordpage.cpp
  settings.cpp
)

# todo: more appropriate icon?
kde4_add_app_icon(scalixadmin_SRCS "${KDE4_ICON_DIR}/oxygen/*/categories/preferences-system-network.png")

kde4_add_executable(scalixadmin ${scalixadmin_SRCS})
TARGET_LINK_LIBRARIES(scalixadmin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KLDAP_LIBS} kdepim )
set_target_properties(scalixadmin PROPERTIES OUTPUT_NAME scalixadmin)

install(TARGETS scalixadmin ${INSTALL_TARGETS_DEFAULT_ARGS} )

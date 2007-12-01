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

kde4_add_executable(scalixadmin ${scalixadmin_SRCS})
TARGET_LINK_LIBRARIES(scalixadmin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdepim kldap )
set_target_properties(scalixadmin PROPERTIES OUTPUT_NAME scalixadmin)

install(TARGETS scalixadmin DESTINATION ${BIN_INSTALL_DIR} )

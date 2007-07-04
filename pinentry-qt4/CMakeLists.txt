project( pinentry-qt4 ) 

set( pinentry-qt4_SRCS
     assuan/assuan-util.c
     assuan/assuan-errors.c
     assuan/assuan-buffer.c
     assuan/assuan-handler.c
     assuan/assuan-listen.c
     assuan/assuan-pipe-server.c
     pinentry/pinentry.c
     qt4/main.cpp
     qt4/pinentrydialog.cpp
     secmem/secmem.c
     secmem/util.c
)

include_directories(
    ${CMAKE_SOURCE_DIR}/pinentry-qt4/assuan
    ${CMAKE_SOURCE_DIR}/pinentry-qt4/secmem
    ${CMAKE_SOURCE_DIR}/pinentry-qt4/pinentry
)

kde4_automoc( ${pinentry-qt4_SRCS} )

kde4_add_executable( pinentry-qt4 ${pinentry-qt4_SRCS} )

target_link_libraries( pinentry-qt4 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

install( TARGETS pinentry-qt4 DESTINATION ${BIN_INSTALL_DIR} )

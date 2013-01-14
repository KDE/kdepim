set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libkleo  )

########### next target ###############

if ( NOT KDEPIM_ONLY_KLEO )
set(test_keylister_SRCS test_keylister.cpp )

kde4_add_executable(test_keylister TEST ${test_keylister_SRCS})

target_link_libraries(test_keylister
    kleo
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QGPGME_LIBRARIES}
    ${QT_QTGUI_LIBRARY} )
endif ()


########### next target ###############

set(test_keygen_SRCS test_keygen.cpp )



kde4_add_executable(test_keygen TEST ${test_keygen_SRCS})

target_link_libraries(test_keygen  kleo ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES})


########### next target ###############

if ( NOT KDEPIM_ONLY_KLEO )
set(test_keyselectiondialog_SRCS test_keyselectiondialog.cpp )

kde4_add_executable(test_keyselectiondialog TEST ${test_keyselectiondialog_SRCS})

target_link_libraries(test_keyselectiondialog kleo ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES} )
endif ()

########### next target ###############

set(test_cryptoconfig_SRCS test_cryptoconfig.cpp )


kde4_add_executable(test_cryptoconfig TEST ${test_cryptoconfig_SRCS})

target_link_libraries(test_cryptoconfig  kleo ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES})

########### next target ###############

set(test_gnupgprocessbase_SRCS test_gnupgprocessbase.cpp )


kde4_add_executable(test_gnupgprocessbase TEST ${test_gnupgprocessbase_SRCS})

target_link_libraries(test_gnupgprocessbase  kleo ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} )

########### next target ###############

set(test_jobs_SRCS test_jobs.cpp )


kde4_add_executable(test_jobs TEST ${test_jobs_SRCS})

target_link_libraries(test_jobs  kleo ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES})

########### next target ###############

kde4_add_executable( test_kdhorizontalline TEST test_kdhorizontalline.cpp )
target_link_libraries( test_kdhorizontalline kleo ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

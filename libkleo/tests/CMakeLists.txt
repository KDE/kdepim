include_directories( ${CMAKE_SOURCE_DIR}/kleo  )


########### next target ###############

set(test_keylister_SRCS test_keylister.cpp )



kde4_add_test_executable(test_keylister ${test_keylister_SRCS})

target_link_libraries(test_keylister  kleo-gpl ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(test_keygen_SRCS test_keygen.cpp )



kde4_add_test_executable(test_keygen ${test_keygen_SRCS})

target_link_libraries(test_keygen  kleo-gpl ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(test_keyselectiondialog_SRCS test_keyselectiondialog.cpp )


kde4_add_test_executable(test_keyselectiondialog ${test_keyselectiondialog_SRCS})

target_link_libraries(test_keyselectiondialog kleo-gpl ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_cryptoconfig_SRCS test_cryptoconfig.cpp )


kde4_add_test_executable(test_cryptoconfig ${test_cryptoconfig_SRCS})

target_link_libraries(test_cryptoconfig  kleo-gpl ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_gnupgprocessbase_SRCS test_gnupgprocessbase.cpp )


kde4_add_test_executable(test_gnupgprocessbase ${test_gnupgprocessbase_SRCS})

target_link_libraries(test_gnupgprocessbase  kleo-gpl ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_jobs_SRCS test_jobs.cpp )


kde4_add_test_executable(test_jobs ${test_jobs_SRCS})

target_link_libraries(test_jobs  kleo-gpl ${KDE4_KDECORE_LIBS} )


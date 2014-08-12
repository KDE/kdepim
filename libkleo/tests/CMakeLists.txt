set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libkleo  )

ecm_add_tests(test_kdhorizontalline.cpp test_jobs.cpp test_gnupgprocessbase.cpp test_cryptoconfig.cpp test_keyselectiondialog.cpp test_keygen.cpp test_keylister.cpp  NAME_PREFIX "kleo-" LINK_LIBRARIES kleo)


include(ECMAddTests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_definitions( -DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )

include_directories (
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_BINARY_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecore/tests
    ${CMAKE_SOURCE_DIR}/templateparser
)
include ( ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_handle_crypto_rpath_for_executable.cmake )

# Convenience macro to add messagecomposer unit tests.
macro( add_messagecomposer_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messagecomposer-${_name})
  target_link_libraries( ${_name} KF5::Mime messagecomposer messageviewer messagecore templateparser KF5::PimTextEdit Qt5::Test    KF5::PimIdentities
  KF5::AkonadiCore
)
endmacro()

macro( add_messagecomposer_cryptotest _source )
  set( _test cryptofunctions.cpp  ${CMAKE_SOURCE_DIR}/messagecore/tests/util.cpp ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messagecomposer-${_name})
  #kde4_handle_crypto_rpath_for_executable( ${_name} )
  target_link_libraries( 
      ${_name}
      KF5::Mime
      messagecomposer
      messageviewer
      messagecore
      kleo
      KF5::PimTextEdit
      KF5::AkonadiCore
      Qt5::Test
      ${QGPGME_LIBRARIES}
      )
endmacro()


# Utility stuff.
add_messagecomposer_test( utiltest.cpp )
add_messagecomposer_test( messagefactorytest.cpp )

# Non-content jobs.
add_messagecomposer_test( skeletonmessagejobtest.cpp )

# Basic content jobs.
add_messagecomposer_test( singlepartjobtest.cpp )
add_messagecomposer_test( multipartjobtest.cpp )

# More complex content jobs.
add_messagecomposer_test( attachmentjobtest.cpp )
add_messagecomposer_test( maintextjobtest.cpp )

# Composer.
add_messagecomposer_test( composertest.cpp )
add_messagecomposer_cryptotest( cryptocomposertest.cpp )

# Crypto
add_messagecomposer_cryptotest( signjobtest.cpp )
add_messagecomposer_cryptotest( encryptjobtest.cpp )
add_messagecomposer_cryptotest( signencrypttest.cpp )

########### next target ###############

set(testkmeditor_SRCS testkmeditor.cpp )

add_executable(testkmeditor ${testkmeditor_SRCS})

target_link_libraries(testkmeditor messagecomposer  KF5::PimTextEdit )

########### next target ###############

set(testkmeditorwin_SRCS testkmeditorwin.cpp )

add_executable(testkmeditorwin ${testkmeditorwin_SRCS})

target_link_libraries(testkmeditorwin messagecomposer  KF5::PimTextEdit )


########### recipientseditor test ###############

set(recipienteditortest_SRCS recipientseditortest.cpp)
add_executable(recipienteditortest ${recipienteditortest_SRCS})
target_link_libraries(recipienteditortest KF5::KIOCore KF5::AkonadiContact messagecomposer kdepim)

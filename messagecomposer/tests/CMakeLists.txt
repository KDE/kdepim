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
  kde4_add_unit_test( ${_name} TESTNAME messagecomposer-${_name} ${_test} )
  target_link_libraries( ${_name} ${KDEPIMLIBS_KMIME_LIBS} messagecomposer messageviewer messagecore templateparser ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
)
endmacro()

macro( add_messagecomposer_cryptotest _source )
  set( _test cryptofunctions.cpp  ${CMAKE_SOURCE_DIR}/messagecore/tests/util.cpp ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagecomposer-${_name} ${_test} )
  kde4_handle_crypto_rpath_for_executable( ${_name} )
  target_link_libraries( 
      ${_name}
      ${KDEPIMLIBS_KMIME_LIBS}
      messagecomposer
      messageviewer
      messagecore
      kleo
      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
      ${KDEPIMLIBS_AKONADI_LIBS}
      ${QT_QTTEST_LIBRARY}
      ${QT_QTGUI_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDE4_KDEUI_LIBS}
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
add_messagecomposer_test( infoparttest.cpp )
add_messagecomposer_test( textparttest.cpp )
add_messagecomposer_test( globalparttest.cpp )

# Crypto
add_messagecomposer_cryptotest( signjobtest.cpp )
add_messagecomposer_cryptotest( encryptjobtest.cpp )
add_messagecomposer_cryptotest( signencrypttest.cpp )

########### next target ###############

set(testkmeditor_SRCS testkmeditor.cpp )

kde4_add_executable(testkmeditor TEST ${testkmeditor_SRCS})

target_link_libraries(testkmeditor messagecomposer ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} )

########### next target ###############

set(testkmeditorwin_SRCS testkmeditorwin.cpp )

kde4_add_executable(testkmeditorwin TEST ${testkmeditorwin_SRCS})

target_link_libraries(testkmeditorwin messagecomposer ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} )


########### recipientseditor test ###############

set(recipienteditortest_SRCS recipientseditortest.cpp)
kde4_add_executable(recipienteditortest TEST ${recipienteditortest_SRCS})
target_link_libraries(recipienteditortest ${KDE4_KIO_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} messagecomposer kdepim)

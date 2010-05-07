set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_definitions( -DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )

include_directories (
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_BINARY_DIR}/messagecomposer
)

# Convenience macro to add messagecomposer unit tests.
macro( add_messagecomposer_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagecomposer-${_name} ${_test} )
  target_link_libraries( ${_name} ${KDE4_KMIME_LIBRARY} messagecomposer messageviewer messagecore ${KDEPIMLIBS_KPIMTEXTEDIT_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS})
endmacro( add_messagecomposer_test )


macro( add_messagecomposer_cryptotest _source )
  set( _test cryptofunctions.cpp ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagecomposer-${_name} ${_test} )
  target_link_libraries( 
      ${_name}
      ${KDE4_KMIME_LIBRARY}
      messagecomposer
      messageviewer
      messagecore
      kleo
      ${KDEPIMLIBS_KPIMTEXTEDIT_LIBRARY}
      ${KDEPIMLIBS_AKONADI_LIBRARY}
      ${QT_QTTEST_LIBRARY}
      ${QT_QTGUI_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      ${KDE4_KDEUI_LIBS}
      ${QT_QT3SUPPORT_LIBRARY}
      ${QGPGME_LIBRARIES}
      )
endmacro( add_messagecomposer_cryptotest )


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
add_messagecomposer_cryptotest( composertest.cpp )

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

target_link_libraries(testkmeditorwin messagecomposer ${KDE4_KDEUI_LIBS} )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories (
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkleo
)

# Convenience macro to add messagecomposer unit tests.
macro( add_messagecomposer_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagecomposer-${_name} ${_test} )
  target_link_libraries( ${_name} ${KDE4_KMIME_LIBRARY} messagecomposer ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} )
endmacro( add_messagecomposer_test )



# Location of the attachments to test.
set( attachments_root "QLatin1String( \"${CMAKE_CURRENT_SOURCE_DIR}/attachments/\" )" )
add_definitions( -DPATH_ATTACHMENTS='${attachments_root}' )

# Utility stuff.
add_messagecomposer_test( utiltest.cpp )

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

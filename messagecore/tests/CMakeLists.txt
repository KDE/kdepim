set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory(gnupg_home)

include_directories( ${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/messagecore )
include ( ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_handle_crypto_rpath_for_executable.cmake )

# Convenience macro to add unit tests.
macro( add_messagecore_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagecore-${_name} ${_test} )
  target_link_libraries( ${_name} messagecore Qt5::Test  kleo )
endmacro()

# Attachment tests.
add_messagecore_test( attachmentcompressjobtest.cpp )
add_messagecore_test( attachmentfrommimecontentjobtest.cpp )
add_messagecore_test( attachmentfromurljobtest.cpp )
add_messagecore_test( attachmentparttest.cpp )
add_messagecore_test( attachmentpropertiesdialogtest.cpp )

# Other tests
add_messagecore_test( stringutiltest.cpp )

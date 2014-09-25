macro(add_texttospeech_unittest _source)
  set(_test ${_source} texttospeechinterfacetest.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(pimcommon-texttospeech-${_name})
  target_link_libraries( ${_name}
    pimcommon
    Qt5::Test
  )
endmacro ()

add_texttospeech_unittest( texttospeechwidgettest.cpp )
add_texttospeech_unittest( texttospeechconfigwidgettest.cpp )

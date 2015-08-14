macro(add_pimcommon_texttospeech_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "pimcommon-texttospeech-"
        LINK_LIBRARIES Qt5::Test pimcommon
    )
endmacro()

add_pimcommon_texttospeech_unittest( texttospeechwidgettest.cpp )
add_pimcommon_texttospeech_unittest( texttospeechconfigwidgettest.cpp )
add_pimcommon_texttospeech_unittest( texttospeechactionstest.cpp )

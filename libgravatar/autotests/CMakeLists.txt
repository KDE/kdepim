macro(add_gravatar_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "gravatar-"
        LINK_LIBRARIES KF5::PimCommon Qt5::Test KF5::Gravatar
    )
endmacro ()

add_gravatar_unittest( gravatarcachetest.cpp )
add_gravatar_unittest( gravatarconfiguresettingsdialogtest.cpp )
add_gravatar_unittest( gravatarconfigwidgettest.cpp )
add_gravatar_unittest( gravatardownloadpixmapdialogtest.cpp )
add_gravatar_unittest( gravatardownloadpixmapwidgettest.cpp )
add_gravatar_unittest( gravatarresolvurljobtest.cpp )


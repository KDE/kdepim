macro(add_pimcommon_gravatar_unittest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "pimcommon-gravatar-"
        LINK_LIBRARIES Qt5::Test pimcommon
    )
endmacro()

add_pimcommon_gravatar_unittest(cachetest gravatarcachetest.cpp)
add_pimcommon_gravatar_unittest(downloadpixmapwidgettest gravatardownloadpixmapwidgettest.cpp ../widgets/gravatardownloadpixmapwidget.cpp ../../pimcommon_debug.cpp)
add_pimcommon_gravatar_unittest(downloadpixmapdialogtest gravatardownloadpixmapdialogtest.cpp ../widgets/gravatardownloadpixmapdialog.cpp ../../pimcommon_debug.cpp)
add_pimcommon_gravatar_unittest(resolveurljobtest gravatarresolvurljobtest.cpp)

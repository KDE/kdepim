add_definitions( -DGRANTLEETHEME_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )
ecm_add_test(grantleethemetest.cpp
    NAME_PREFIX "grantleetheme-"
    LINK_LIBRARIES Qt5::Test KF5::GrantleeTheme
)

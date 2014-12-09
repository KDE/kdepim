add_definitions( -DGRANTLEETHEME_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )


set( grantlee_theme_source grantleethemetest.cpp ../grantleetheme.cpp)
add_executable( grantleethemetest ${grantlee_theme_source})
add_test(grantleethemetest grantleethemetest)
ecm_mark_as_test(grantleethemetest)
target_link_libraries( grantleethemetest Qt5::Test grantleetheme)


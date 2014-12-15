include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)

set( shorturlenginetest_source shorturlenginestest.cpp )
add_executable( shorturlenginetest ${shorturlenginetest_source})
add_test(shorturlenginetest shorturlenginetest)
ecm_mark_as_test(shorturlenginetest)
target_link_libraries( shorturlenginetest Qt5::Test KF5::KDELibs4Support Qt5::Gui pimcommon)

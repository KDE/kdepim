include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)

set( shorturlenginetest_source shorturlenginestest.cpp )
kde4_add_unit_test( shorturlenginetest ${shorturlenginetest_source})
target_link_libraries( shorturlenginetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} pimcommon)

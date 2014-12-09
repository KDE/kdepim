add_definitions( -DGRANTLEETHEME_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )


set( grantlee_theme_source grantleethemetest.cpp ../grantleetheme.cpp)
kde4_add_unit_test( grantleethemetest ${grantlee_theme_source})
target_link_libraries( grantleethemetest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})


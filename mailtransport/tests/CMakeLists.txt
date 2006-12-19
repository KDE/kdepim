if ( KDE4_BUILD_TESTS )

set(tm_srcs transportmgr.cpp)
kde4_automoc(${tm_srcs})
kde4_add_executable(transportmgr ${tm_srcs})
target_link_libraries(transportmgr ${KDE4_KDEUI_LIBS} mailtransport)

endif ( KDE4_BUILD_TESTS )

project(blogilo)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

find_package(KF5Blog ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)

add_subdirectory( src )
add_subdirectory( icons )

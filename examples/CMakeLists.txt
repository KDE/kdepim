

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

find_package(KF5MicroBlog ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)

add_subdirectory(akonablog)
add_subdirectory(akonalendar)
add_subdirectory(etm_usage)
add_subdirectory(kabc)
add_subdirectory(mailreader)
add_subdirectory(coisceim)


set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )

macro_optional_add_subdirectory(akonablog)
macro_optional_add_subdirectory(akonalendar)
macro_optional_add_subdirectory(etm_usage)
macro_optional_add_subdirectory(kabc)
macro_optional_add_subdirectory(mailreader)
macro_optional_add_subdirectory(coisceim)


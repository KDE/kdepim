set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

macro_optional_add_subdirectory(akonablog)
macro_optional_add_subdirectory(akonalendar)
macro_optional_add_subdirectory(etm_usage)
macro_optional_add_subdirectory(kabc)
macro_optional_add_subdirectory(mailreader)


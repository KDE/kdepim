set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

macro_optional_add_subdirectory(akonablog)
macro_optional_add_subdirectory(mailreader)
macro_optional_add_subdirectory(kabc)

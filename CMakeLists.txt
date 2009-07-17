project(krss)

if (KRSS_ENABLE_PROTOBUF_SERIALIZER)
    find_package(ProtocolBuffers REQUIRED)
    add_definitions( -DKRSS_ENABLE_PROTOBUF_SERIALIZER)
    include_directories( ${PROTOBUF_INCLUDE_DIR} )
endif (KRSS_ENABLE_PROTOBUF_SERIALIZER)

include_directories( ${Boost_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory(krss)
add_subdirectory(krssresource)
add_subdirectory(plugin)
add_subdirectory(resources)
add_subdirectory(cmake/modules)

project(krss)

#kdelibs
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

#kdepimlibs
find_package(KdepimLibs 4.5.85 REQUIRED)
include_directories(${KDEPIMLIBS_INCLUDE_DIRS})

#akonadi
find_package(Akonadi REQUIRED)
include_directories(${AKONADI_INCLUDE_DIR})

# Boost
find_package(Boost REQUIRED)


if (KRSS_ENABLE_PROTOBUF_SERIALIZER)
    find_package(ProtocolBuffers REQUIRED)
    add_definitions( -DKRSS_ENABLE_PROTOBUF_SERIALIZER)
    include_directories( ${PROTOBUF_INCLUDE_DIR} )
endif (KRSS_ENABLE_PROTOBUF_SERIALIZER)

include_directories( ${Boost_INCLUDE_DIR} ${QT_INCLUDES} ${krss_SOURCE_DIR} ${krss_BINARY_DIR} ${KDE4_INCLUDES})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII" )
add_subdirectory(krss)
add_subdirectory(krssresource)
add_subdirectory(plugin)
add_subdirectory(resources)
add_subdirectory(examples)
add_subdirectory(cmake/modules)

 
find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories( ${KDE4_INCLUDES} )

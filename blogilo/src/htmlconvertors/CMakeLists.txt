include_directories( ${KDE4_INCLUDES} )

set(htmlconvertor_SRCS
 bilbocssparser.cpp
 bilbohtmlparser.cpp
 bilbotexthtmlimporter.cpp
 htmlexporter.cpp
)

kde4_add_library( htmlconvertors STATIC ${htmlconvertor_SRCS} )

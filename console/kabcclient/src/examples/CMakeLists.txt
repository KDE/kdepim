project(kabcclient_examples)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${QT_INCLUDE_DIR}
${KDE_INCLUDE_DIR} )

########### next target ###############

SET(converter_SRCS
    converter.cpp
    ../csvtemplate.cpp
    ../csvtemplatefactory.cpp
    ../formatfactory.cpp
    ../inputformatimpls.cpp
    ../outputformatimpls.cpp)

add_executable(converter ${converter_SRCS})

target_link_libraries(converter ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS})


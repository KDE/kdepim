set(icsendanalyzer_SRCS icsendanalyzer.cpp )

include_directories(${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})
# kde4_add_library just defines MAKE_ICS_LIB which is not really unique
add_definitions(-DMAKE_PIMSTRIGIANALYZER_LIB)

if(WINCE)
kde4_add_library(ics STATIC ${icsendanalyzer_SRCS})
else()
kde4_add_library(ics MODULE ${icsendanalyzer_SRCS})
endif()

target_link_libraries(ics ${STRIGI_STREAMANALYZER_LIBRARY} ${STRIGI_STREAMS_LIBRARY} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDE4_KIO_LIBS} )

set_target_properties(ics PROPERTIES PREFIX strigiea_)
install(TARGETS ics LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi
                    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}/strigi)

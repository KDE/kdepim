set(ctgendanalyzer_SRCS ctgendanalyzer.cpp )

include_directories(${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})
add_definitions(-DMAKE_PIMSTRIGIANALYZER_LIB)

if(WINCE)
kde4_add_library(ctg STATIC ${ctgendanalyzer_SRCS})
else()
kde4_add_library(ctg MODULE ${ctgendanalyzer_SRCS})
endif()

target_link_libraries(ctg  ${QT_QTCORE_LIBRARY} ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${KDEPIMLIBS_KABC_LIBS})
if(NOT WINCE)
target_link_libraries(ctg ${KDE4_KFILE_LIBS})
endif()
set_target_properties(ctg PROPERTIES PREFIX strigiea_)
install(TARGETS ctg LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi
                    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}/strigi)

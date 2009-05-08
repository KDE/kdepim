set(vcfendanalyzer_SRCS vcfendanalyzer.cpp )

include_directories(${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})
# kde4_add_library just defines MAKE_VCF_LIB which is not really unique
add_definitions(-DMAKE_PIMSTRIGIANALYZER_LIB)

kde4_add_library(vcf MODULE ${vcfendanalyzer_SRCS})

target_link_libraries(vcf ${KDE4_KFILE_LIBS} ${QT_QTCORE_LIBRARY} ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${KDEPIMLIBS_KABC_LIBS})
set_target_properties(vcf PROPERTIES PREFIX strigiea_)
install(TARGETS vcf LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

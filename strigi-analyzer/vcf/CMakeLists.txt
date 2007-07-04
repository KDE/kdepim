set(vcfendanalyzer_SRCS vcfendanalyzer.cpp )

include_directories(${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})

kde4_add_library(vcf MODULE ${vcfendanalyzer_SRCS})

target_link_libraries(vcf ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KABC_LIBS} ${QT_QTCORE_LIBRARY})
set_target_properties(vcf PROPERTIES PREFIX strigiea_)
install(TARGETS vcf LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

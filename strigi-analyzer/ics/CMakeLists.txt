set(icsendanalyzer_SRCS icsendanalyzer.cpp )

include_directories(${STRIGI_INCLUDE_DIR})

kde4_add_library(ics MODULE ${icsendanalyzer_SRCS})

target_link_libraries(ics ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KIO_LIBS} )
set_target_properties(ics PROPERTIES PREFIX strigiea_)
install(TARGETS ics LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


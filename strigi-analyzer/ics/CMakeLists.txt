set(kfile_ics_SRCS kfile_ics.cpp )

kde4_add_library(ics MODULE ${kfile_ics_SRCS})

target_link_libraries(ics ${STREAMANALYZER_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KIO_LIBS} )
set_target_properties(ics PROPERTIES PREFIX strigiea_)
install(TARGETS ics LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


kde4_add_library(rfc822 MODULE rfc822endanalyzer.cpp )
target_link_libraries(rfc822 ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KIO_LIBS})
set_target_properties(rfc822 PROPERTIES PREFIX strigiea_)
install(TARGETS rfc822 LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

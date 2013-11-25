find_program(DBLATEX_EXECUTABLE dblatex)
add_feature_info("Kontact Touch license information" DBLATEX_EXECUTABLE "Dblatex is needed for building the Kontact Touch license information.")

if(DBLATEX_EXECUTABLE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/licenses.pdf
    COMMAND ${DBLATEX_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/licenses.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/licenses.xml
  )

  add_custom_target(licences_pdf DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/licenses.pdf)
endif()

install(FILES licenses.pdf DESTINATION ${DATA_INSTALL_DIR}/kontact-touch/)
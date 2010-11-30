find_program(DBLATEX_EXECUTABLE dblatex)
macro_log_feature(DBLATEX_EXECUTABLE "dblatex" "The DocBook to LaTeX converter" "http://dblatex.sourceforge.net/" FALSE "" "Required for generating Kontact-Touch license information.")

if (DBLATEX_EXECUTABLE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/licenses.pdf
    COMMAND ${DBLATEX_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/licenses.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/licenses.xml
  )

  add_custom_target(licences_pdf DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/licenses.pdf)

endif (DBLATEX_EXECUTABLE)

install(FILES licenses.pdf DESTINATION ${DATA_INSTALL_DIR}/kontact-touch/)

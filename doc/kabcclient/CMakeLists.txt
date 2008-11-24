kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)
########### install files ###############
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kabcclient.1 DESTINATION "${MAN_INSTALL_DIR}/man1")

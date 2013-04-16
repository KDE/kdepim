project(linkgrammarchecker)

include_directories(${LinkGrammar_INCLUDE_DIR})
set( linkgrammarchecker_SRCS grammarlinkclient.cpp grammarlinkplugin.cpp)

kde4_add_plugin(grammar_link ${linkgrammarchecker_SRCS})

target_link_libraries(grammar_link  ${KDE4_KDECORE_LIBS} ${LinkGrammar_LIBRARIES} grammar)

install(TARGETS grammar_link  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES grammar_link.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )



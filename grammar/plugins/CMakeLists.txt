project(plugins)
find_package(LinkGrammar)
set_package_properties(LinkGrammar PROPERTIES DESCRIPTION "Grammar checking Library"
                                              URL "http://www.abisource.com/projects/link-grammar/"
                                              TYPE OPTIONAL
                                              PURPOSE "Required by Grammar-Check plugin"
                      )

if(LINKGRAMMAR_FOUND)
  add_subdirectory(linkgrammar)
endif()


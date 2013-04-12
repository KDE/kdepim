project(plugins)
macro_optional_find_package(LinkGrammar)
macro_log_feature(LINK_GRAMMAR_FOUND "link-grammar" "Grammar checking Library" "http://www.abisource.com/projects/link-grammar/" FALSE "" "Required by Grammar-Check plugin")
if (LINK_GRAMMAR_FOUND)
    macro_optional_add_subdirectory( linkgrammar )
endif (LINK_GRAMMAR_FOUND)


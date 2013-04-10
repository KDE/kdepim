project(grammar)

macro_optional_find_package(LinkGrammar)
macro_log_feature(LINK_GRAMMAR_FOUND "link-grammar" "Grammar checking Library" "http://www.abisource.com/projects/link-grammar/" FALSE "" "Required by Grammar-Check plugin")
IF (LINK_GRAMMAR_FOUND)
    macro_optional_add_subdirectory( grammarcheck )
ENDIF (LINK_GRAMMAR_FOUND)


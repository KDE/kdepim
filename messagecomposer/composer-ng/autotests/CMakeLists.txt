macro(add_messagecomposer_ng_test _source)
    ecm_add_test(${_source}
        NAME_PREFIX "messagecomposer-composerng-"
        LINK_LIBRARIES Qt5::Test pimcommon messagecomposer KF5::XmlGui
    )
endmacro()

add_messagecomposer_ng_test(richtextcomposertest.cpp)
add_messagecomposer_ng_test(richtextcomposercontrolertest.cpp)
add_messagecomposer_ng_test(richtextcomposeractionstest.cpp)
add_messagecomposer_ng_test(richtextexternalcomposertest.cpp)
add_messagecomposer_ng_test(richtextcomposerimagestest.cpp)
add_messagecomposer_ng_test(richtextcomposersignaturestest.cpp)

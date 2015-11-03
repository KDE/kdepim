macro(add_mailcommon_filter_test _name)
    ecm_add_test(${ARGN} 
        TEST_NAME ${_name}
        NAME_PREFIX "mailcommon-filter-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore KF5::AkonadiMime
                       KF5::Libkdepim KF5::MailCommon KF5::PimCommon Phonon::phonon4qt5 KF5::KIOWidgets KF5::IconThemes KF5::TemplateParser KF5::MailTransport 
    )
endmacro()

set(filter_common_SRCS)
ecm_qt_declare_logging_category(filter_common_SRCS HEADER mailcommon_debug.h IDENTIFIER MAILCOMMON_LOG CATEGORY_NAME log_mailcommon)

add_mailcommon_filter_test(itemcontexttest
    itemcontexttest.cpp
    ../itemcontext.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionpipethroughtest
   filteractionpipethroughtest.cpp
   ../filteractions/filteractionpipethrough.cpp
   ../filteractions/filteractionwithcommand.cpp
   ../filteractions/filteractionwithurl.cpp
   ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionheadertest
    filteractionaddheadertest.cpp
    ../filteractions/filteractionaddheader.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractiondeletetest
    filteractiondeletetest.cpp
    ../filteractions/filteractiondelete.cpp
    ../filteractions/filteractionwithnone.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionrewriteheadertest
    filteractionrewriteheadertest.cpp
    ../filteractions/filteractionrewriteheader.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionsetstatustest
    filteractionsetstatustest.cpp
    ../filteractions/filteractionsetstatus.cpp
    ../filteractions/filteractionstatus.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionreplytotest
    filteractionreplytotest.cpp
    ../filteractions/filteractionreplyto.cpp
    ../filteractions/filteractionwithaddress.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionwithurltest
    filteractionwithurltest.cpp
    ../filteractions/filteractionwithurl.cpp
    ../filteractions/filteractionwithaddress.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionremoveheadertest
    filteractionremoveheadertest.cpp
    ../filteractions/filteractionremoveheader.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionwithaddresstest
    filteractionwithaddresstest.cpp
    ../filteractions/filteractionwithaddress.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionaddtoaddressbooktest
    filteractionaddtoaddressbooktest.cpp
    ../filteractions/filteractionaddtoaddressbook.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionplaysoundtest
    filteractionplaysoundtest.cpp
    ../filteractions/filteractionplaysound.cpp
    ../filteractions/filteractionwithtest.cpp
    ../filteractions/filteractionplaysound.cpp
    ../soundtestwidget.cpp
    ../dialog/filteractionmissingargumentdialog.cpp
    ../kmfilteraccountlist.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(invalidfilterdialogtest
    invalidfilterdialogtest.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(invalidfilterwidgettest
    invalidfilterwidgettest.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(invalidfilterlistwidgettest
    invalidfilterlistwidgettest.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionunsetstatustest
    filteractionunsetstatustest.cpp
    ../filteractions/filteractionunsetstatus.cpp
    ../filteractions/filteractionstatus.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionredirecttest
    filteractionredirecttest.cpp
    ../filteractions/filteractionredirect.cpp
    ../filteractions/filteractionwithaddress.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(invalidfilterinfotest
    invalidfilterinfotest.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionsendfakedispositiontest
    filteractionsendfakedispositiontest.cpp
    ../filteractions/filteractionsendfakedisposition.cpp
    ../filteractions/filteractionwithstringlist.cpp
    ../filteractions/filteractionwithstring.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionforwardtest
    filteractionforwardtest.cpp
    ../filteractions/filteractionforward.cpp
    ../filteractions/filteractionwithaddress.cpp
    ../filteractions/filteractionwithstring.cpp
    ../dialog/filteractionmissingargumentdialog.cpp
    ../kmfilteraccountlist.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(invalidfilterinfowidgettest
    invalidfilterinfowidgettest.cpp
    ../invalidfilters/invalidfilterinfowidget.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractioncopytest
    filteractioncopytest.cpp
    ../filteractions/filteractioncopy.cpp
    ../filteractions/filteractionwithfolder.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionmovetest
    filteractionmovetest.cpp
    ../filteractions/filteractionmove.cpp
    ../filteractions/filteractionwithfolder.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionexectest
    filteractionexectest.cpp
    ../filteractions/filteractionexec.cpp
    ../filteractions/filteractionwithcommand.cpp
    ../filteractions/filteractionwithurl.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionsendreceipttest
    filteractionsendreceipttest.cpp
    ../filteractions/filteractionsendreceipt.cpp
    ../filteractions/filteractionwithnone.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filteractionsettransporttest
    filteractionsettransporttest.cpp
    ../filteractions/filteractionsettransport.cpp
    ../dialog/filteractionmissingargumentdialog.cpp
    ../kmfilteraccountlist.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(filterconverttosieveresultdialog
    filterconverttosieveresultdialogtest.cpp
    ../filterconverter/filterconverttosieveresultdialog.cpp
    ${filter_common_SRCS}
)

add_mailcommon_filter_test(mailfiltertest
    mailfiltertest.cpp
    ${filter_common_SRCS}
)

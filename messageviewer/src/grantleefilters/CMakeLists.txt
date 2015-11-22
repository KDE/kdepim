#project(grantleefilters)

set(grantlee_messageheaderfilters_SRCS
    messageheadergrantleefilters.cpp
    messageheaderfilter.cpp
)

ecm_qt_declare_logging_category(grantlee_messageheaderfilters_SRCS HEADER grantlee_messageheaderfilter_debug.h IDENTIFIER GRANTLEEMESSAGEVIEWERFILTER_LOG CATEGORY_NAME log_grantlee_messageviewerheader)

add_library(grantlee_messageheaderfilters MODULE ${grantlee_messageheaderfilters_SRCS})
GRANTLEE_ADJUST_PLUGIN_NAME(grantlee_messageheaderfilters)

target_link_libraries(grantlee_messageheaderfilters KF5::MessageCore Grantlee5::TextDocument Grantlee5::Templates)

install(TARGETS grantlee_messageheaderfilters LIBRARY DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR} )


project(messagecore)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"libmessagecore\")

########### next target ###############

set(messagecore_attachment_LIB_SRCS
   attachment/attachmentcompressjob.cpp
   attachment/attachmentfromfolderjob.cpp
   attachment/attachmentfrommimecontentjob.cpp
   attachment/attachmentfromurlbasejob.cpp
   attachment/attachmentfromurljob.cpp
   attachment/attachmentloadjob.cpp
   attachment/attachmentpart.cpp
   attachment/attachmentpropertiesdialog.cpp
   attachment/attachmentupdatejob.cpp
   attachment/attachmentfromurlutils.cpp
)

set(messagecore_misc_LIB_SRCS
   misc/imagecollector.cpp
   misc/mailinglist.cpp
   misc/mdnstateattribute.cpp
)


set(messagecore_helper_LIB_SRCS
   helpers/messagehelpers.cpp
   helpers/nodehelper.cpp
)

set(messagecore_LIB_SRCS
   ${messagecore_attachment_LIB_SRCS}
   ${messagecore_misc_LIB_SRCS}
   ${messagecore_helper_LIB_SRCS}
   settings/messagecoresettings.cpp
   utils/stringutil.cpp
)

kconfig_add_kcfg_files(messagecore_LIB_SRCS
  settings/globalsettings_messagecore.kcfgc
)

ki18n_wrap_ui(messagecore_LIB_SRCS
  attachment/ui/attachmentpropertiesdialog.ui
  attachment/ui/attachmentpropertiesdialog_readonly.ui
)

ecm_qt_declare_logging_category(messagecore_LIB_SRCS HEADER messagecore_debug.h IDENTIFIER MESSAGECORE_LOG CATEGORY_NAME log_messagecore)

add_library(messagecore ${messagecore_LIB_SRCS})
generate_export_header(messagecore BASE_NAME messagecore)

target_link_libraries(messagecore
PUBLIC
  KF5::Mime
PRIVATE
  KF5::KIOCore
  KF5::Codecs
  KF5::AkonadiCore
  KF5::PimTextEdit
  KF5::AkonadiMime
  KF5::Archive
  KF5::ConfigWidgets
  KF5::Completion
  KF5::IconThemes
  Qt5::Network
)

target_include_directories(messagecore PUBLIC "$<BUILD_INTERFACE:${messagecore_SOURCE_DIR};${messagecore_BINARY_DIR}>")

set_target_properties(messagecore PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS messagecore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

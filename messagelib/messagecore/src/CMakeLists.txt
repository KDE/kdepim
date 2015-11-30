add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

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
)


set(messagecore_helper_LIB_SRCS
   helpers/messagehelpers.cpp
   helpers/nodehelper.cpp
)

set(messagecore_LIB_SRCS
   ${messagecore_attachment_LIB_SRCS}
   ${messagecore_misc_LIB_SRCS}
   ${messagecore_helper_LIB_SRCS}
   utils/stringutil.cpp
   messagecoreutil.cpp
   settings/messagecoresettings.cpp
)

kconfig_add_kcfg_files(messagecore_LIB_SRCS
  settings/globalsettings_messagecore.kcfgc
)

ki18n_wrap_ui(messagecore_LIB_SRCS
  attachment/ui/attachmentpropertiesdialog.ui
  attachment/ui/attachmentpropertiesdialog_readonly.ui
)

ecm_qt_declare_logging_category(messagecore_LIB_SRCS HEADER messagecore_debug.h IDENTIFIER MESSAGECORE_LOG CATEGORY_NAME log_messagecore)

add_library(KF5MessageCore ${messagecore_LIB_SRCS})
generate_export_header(KF5MessageCore BASE_NAME messagecore)
add_library(KF5::MessageCore ALIAS KF5MessageCore)

target_link_libraries(KF5MessageCore
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
  KF5::I18n
)

target_include_directories(KF5MessageCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/MessageCore/;${KDE_INSTALL_INCLUDEDIR_KF5}/messagecore>")
target_include_directories(KF5MessageCore PUBLIC "$<BUILD_INTERFACE:${messagecore_SOURCE_DIR}/src;${messagecore_BINARY_DIR}/src;>")

set_target_properties(KF5MessageCore PROPERTIES
    VERSION ${MESSAGECORE_VERSION_STRING}
    SOVERSION ${MESSAGECORE_SOVERSION}
    EXPORT_NAME MessageCore
)

install(TARGETS
    KF5MessageCore
    EXPORT KF5MessageCoreTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)


ecm_generate_headers(MessageCore_Camelcasemain_HEADERS
  HEADER_NAMES
  MessageCoreUtil

  REQUIRED_HEADERS MessageCore_main_HEADERS
  PREFIX MessageCore
)


ecm_generate_headers(MessageCore_Camelcasemisc_HEADERS
  HEADER_NAMES
  ImageCollector
  MailingList

  REQUIRED_HEADERS MessageCore_misc_HEADERS
  PREFIX MessageCore
  RELATIVE misc
)

ecm_generate_headers(MessageCore_Camelcaseutils_HEADERS
  HEADER_NAMES
  StringUtil
  REQUIRED_HEADERS MessageCore_utils_HEADERS
  PREFIX MessageCore
  RELATIVE utils
)

ecm_generate_headers(MessageCore_Camelcasesettings_HEADERS
  HEADER_NAMES
  MessageCoreSettings
  REQUIRED_HEADERS MessageCore_settings_HEADERS
  PREFIX MessageCore
  RELATIVE settings
)


ecm_generate_headers(MessageCore_Camelcaseattachment_HEADERS
  HEADER_NAMES
  AttachmentFromMimeContentJob
  AttachmentFromUrlBaseJob
  AttachmentFromUrlJob
  AttachmentLoadJob
  AttachmentPart
  AttachmentPropertiesDialog
  AttachmentCompressJob
  AttachmentFromFolderJob
  AttachmentUpdateJob
  AttachmentFromUrlUtils

  REQUIRED_HEADERS MessageCore_attachment_HEADERS
  PREFIX MessageCore
  RELATIVE attachment
)

ecm_generate_headers(MessageCore_Camelcasehelpers_HEADERS
  HEADER_NAMES
  NodeHelper
  MessageHelpers
  REQUIRED_HEADERS MessageCore_helpers_HEADERS
  PREFIX MessageCore
  RELATIVE helpers
)

ecm_generate_pri_file(BASE_NAME MessageCore
    LIB_NAME KF5MessageCore
    DEPS "Mime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/MessageCore
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})
install(FILES
    ${MessageCore_Camelcasehelpers_HEADERS}
    ${MessageCore_Camelcasesettings_HEADERS}
    ${MessageCore_Camelcaseutils_HEADERS}
    ${MessageCore_Camelcaseattachment_HEADERS}
    ${MessageCore_Camelcasemisc_HEADERS}
    ${MessageCore_Camelcasemain_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/MessageCore
    COMPONENT Devel
)

install(FILES
    ${MessageCore_HEADERS}
    ${MessageCore_helpers_HEADERS}
    ${MessageCore_settings_HEADERS}
    ${MessageCore_utils_HEADERS}
    ${MessageCore_main_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/messagecore_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_messagecore.h
    ${MessageCore_attachment_HEADERS}
    ${MessageCore_misc_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/messagecore
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

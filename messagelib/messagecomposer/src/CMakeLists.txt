

add_definitions(-DTRANSLATION_DOMAIN=\"libmessagecomposer\")


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if(BUILD_TESTING)
   add_subdirectory( imagescaling/autotests )
   add_subdirectory( imagescaling/tests )
endif()


set( messagecomposer_job_src
     job/jobbase.cpp
     job/contentjobbase.cpp
     job/attachmentjob.cpp
     job/singlepartjob.cpp
     job/multipartjob.cpp
     job/maintextjob.cpp
     job/signjob.cpp
     job/encryptjob.cpp
     job/signencryptjob.cpp
     job/transparentjob.cpp
     job/inserttextfilejob.cpp
     job/skeletonmessagejob.cpp
     job/aliasesexpandjob.cpp
     job/emailaddressresolvejob.cpp
     job/attachmentfrompublickeyjob.cpp
     job/distributionlistexpandjob.cpp
     job/savecontactpreferencejob.cpp
     job/attachmentvcardfromaddressbookjob.cpp
)

set( messagecomposer_composer_src
     composer/composer.cpp
     composer/signaturecontroller.cpp
     composer/composerlineedit.cpp
     composer/composerviewbase.cpp
     composer/keyresolver.cpp
)

set( messagecomposer_recipient_src
     recipient/recipientspicker.cpp
     recipient/recipient.cpp
     recipient/recipientline.cpp
     recipient/recipientseditor.cpp
     recipient/recipientseditorsidewidget.cpp
     recipient/kwindowpositioner.cpp
     recipient/distributionlistdialog.cpp
)

set( messagecomposer_imagescaling_src
     imagescaling/imagescaling.cpp
     imagescaling/imagescalingwidget.cpp
     imagescaling/imagescalingutils.cpp
     imagescaling/imagescalingselectformat.cpp
)

set( messagecomposer_part_src
     part/messagepart.cpp
     part/globalpart.cpp
     part/infopart.cpp
     part/textpart.cpp
)

set( messagecomposer_attachment_src
     attachment/attachmentcontrollerbase.cpp
     attachment/attachmentmodel.cpp
)

set( messagecomposer_helper_src
     helper/messagehelper.cpp
     helper/messagefactory.cpp
)

set( messagecomposer_sender_src
     sender/akonadisender.cpp
)

set(messagecomposer_followupreminder_SRCS
     followupreminder/followupreminderselectdatedialog.cpp
     followupreminder/followupremindercreatejob.cpp
)

set(messagecomposer_richtextcomposerng_SRCS
     composer-ng/richtextcomposerng.cpp
     composer-ng/richtextcomposersignatures.cpp
)

set( messagecomposer_src
     ${messagecomposer_richtextcomposerng_SRCS}
     ${messagecomposer_part_src}
     ${messagecomposer_imagescaling_src}
     ${messagecomposer_job_src}
     ${messagecomposer_composer_src}
     ${messagecomposer_recipient_src}
     ${messagecomposer_attachment_src}
     ${messagecomposer_helper_src}
     ${messagecomposer_sender_src}
     ${messagecomposer_followupreminder_SRCS}
     utils/util.cpp
     settings/messagecomposersettings.cpp
)

ki18n_wrap_ui(messagecomposer_src
     imagescaling/ui/imagescalingwidget.ui
)
ecm_qt_declare_logging_category(messagecomposer_src HEADER messagecomposer_debug.h IDENTIFIER MESSAGECOMPOSER_LOG CATEGORY_NAME log_messagecomposer)


if(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT true)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT true)
else()
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT false)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT false)
endif()

configure_file(settings/messagecomposer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messagecomposer.kcfg)

kconfig_add_kcfg_files(messagecomposer_src
  settings/messagecomposersettings_base.kcfgc 
)

add_library( KF5MessageComposer ${messagecomposer_src} )
generate_export_header(KF5MessageComposer BASE_NAME messagecomposer)
add_library(KF5::MessageComposer ALIAS KF5MessageComposer)
target_link_libraries(KF5MessageComposer
PUBLIC
  KF5::Mime
  KF5::MessageCore
  KF5::PimCommon
  KF5::AkonadiCore
  KF5::IdentityManagement
  KF5::AkonadiMime
  KF5::Libkleo
PRIVATE
  KF5::MailTransport
  KF5::PimTextEdit
  KF5::TemplateParser
  KF5::MessageViewer
  KF5::AkonadiWidgets
  KF5::Libkdepim
  KF5::CalendarCore
  KF5::KIOCore
  Grantlee5::TextDocument
  KF5::SendLater
  KF5::FollowupReminder
  KF5::Archive
  KF5::Ldap
)

target_include_directories(KF5MessageComposer INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/MessageComposer/;${KDE_INSTALL_INCLUDEDIR_KF5}/messagecomposer>")

target_include_directories(KF5MessageComposer PUBLIC "$<BUILD_INTERFACE:${messagecomposer_SOURCE_DIR}/src;${messagecomposer_BINARY_DIR}/src>")


set_target_properties(KF5MessageComposer PROPERTIES
    VERSION ${MESSAGECOMPOSER_VERSION_STRING}
    SOVERSION ${MESSAGECOMPOSER_SOVERSION}
    EXPORT_NAME MessageComposer
)

install(TARGETS
    KF5MessageComposer
    EXPORT KF5MessageComposerTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)


ecm_generate_headers(MessageComposer_Camelcaseattachement_HEADERS
  HEADER_NAMES
  AttachmentModel
  AttachmentControllerBase
  REQUIRED_HEADERS MessageComposer_attachement_HEADERS
  PREFIX MessageComposer
  RELATIVE attachment
)

ecm_generate_headers(MessageComposer_Camelcasecomposer_HEADERS
  HEADER_NAMES
  Composer
  ComposerLineEdit
  ComposerViewBase
  SignatureController
  REQUIRED_HEADERS MessageComposer_composer_HEADERS
  PREFIX MessageComposer
  RELATIVE composer
)

ecm_generate_headers(MessageComposer_Camelcasecomposerng_HEADERS
  HEADER_NAMES
  RichTextComposerNg
  RichTextComposerSignatures
  REQUIRED_HEADERS MessageComposer_composerng_HEADERS
  PREFIX MessageComposer
  RELATIVE composer-ng
)

ecm_generate_headers(MessageComposer_Camelcasesender_HEADERS
  HEADER_NAMES
  AkonadiSender
  MessageSender
  REQUIRED_HEADERS MessageComposer_sender_HEADERS
  PREFIX MessageComposer
  RELATIVE sender
)

ecm_generate_headers(MessageComposer_Camelcaseutils_HEADERS
  HEADER_NAMES
  Util
  Kleo_Util
  REQUIRED_HEADERS MessageComposer_utils_HEADERS
  PREFIX MessageComposer
  RELATIVE utils
)

ecm_generate_headers(MessageComposer_Camelcasehelper_HEADERS
  HEADER_NAMES
  MessageFactory
  MessageHelper
  REQUIRED_HEADERS MessageComposer_helper_HEADERS
  PREFIX MessageComposer
  RELATIVE helper
)

ecm_generate_headers(MessageComposer_Camelcasesettings_HEADERS
  HEADER_NAMES
  MessageComposerSettings
  REQUIRED_HEADERS MessageComposer_settings_HEADERS
  PREFIX MessageComposer
  RELATIVE settings
)

ecm_generate_headers(MessageComposer_Camelcasepart_HEADERS
  HEADER_NAMES
  TextPart
  GlobalPart
  InfoPart
  MessagePart
  REQUIRED_HEADERS MessageComposer_part_HEADERS
  PREFIX MessageComposer
  RELATIVE part
)

ecm_generate_headers(MessageComposer_Camelcasefollowupreminder_HEADERS
  HEADER_NAMES
  FollowupReminderCreateJob
  FollowUpReminderSelectDateDialog
  REQUIRED_HEADERS MessageComposer_followupreminder_HEADERS
  PREFIX MessageComposer
  RELATIVE followupreminder
)

ecm_generate_headers(MessageComposer_Camelcaserecipient_HEADERS
  HEADER_NAMES
  Recipient
  RecipientsEditor
  DistributionListDialog
  RecipientLine
  REQUIRED_HEADERS MessageComposer_recipient_HEADERS
  PREFIX MessageComposer
  RELATIVE recipient
)

ecm_generate_headers(MessageComposer_Camelcaseimagescaling_HEADERS
  HEADER_NAMES
  ImageScalingWidget
  REQUIRED_HEADERS MessageComposer_imagescaling_HEADERS
  PREFIX MessageComposer
  RELATIVE imagescaling
)

ecm_generate_headers(MessageComposer_Camelcasejob_HEADERS
  HEADER_NAMES
  InsertTextFileJob
  AttachmentJob
  SinglepartJob
  MainTextJob
  AttachmentFromPublicKeyJob
  MultipartJob
  EncryptJob
  AttachmentVcardFromAddressBookJob
  SignJob
  TransparentJob
  JobBase
  AliasesExpandJob
  SkeletonMessageJob
  REQUIRED_HEADERS MessageComposer_job_HEADERS
  PREFIX MessageComposer
  RELATIVE job
)
ecm_generate_pri_file(BASE_NAME MessageComposer
    LIB_NAME KF5MessageComposer
    DEPS "Mime MessageCore PimCommon Akonadi IdentityManagement AkonadiMime Libkleo" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/MessageComposer
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})
install(FILES
    ${MessageComposer_Camelcasecomposer_HEADERS}
    ${MessageComposer_Camelcasecomposerng_HEADERS}
    ${MessageComposer_Camelcasesender_HEADERS}
    ${MessageComposer_Camelcaseutils_HEADERS}
    ${MessageComposer_Camelcasehelper_HEADERS}
    ${MessageComposer_Camelcasesettings_HEADERS}
    ${MessageComposer_Camelcasepart_HEADERS}
    ${MessageComposer_Camelcasefollowupreminder_HEADERS}
    ${MessageComposer_Camelcaserecipient_HEADERS}
    ${MessageComposer_Camelcaseimagescaling_HEADERS}
    ${MessageComposer_Camelcasejob_HEADERS}
    ${MessageComposer_Camelcaseattachement_HEADERS}

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/MessageComposer
    COMPONENT Devel
)

install(FILES
    ${MessageComposer_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/messagecomposer_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/messagecomposersettings_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/messagecomposer_debug.h
    ${MessageComposer_composer_HEADERS}
    ${MessageComposer_composerng_HEADERS}
    ${MessageComposer_sender_HEADERS}
    ${MessageComposer_utils_HEADERS}
    ${MessageComposer_helper_HEADERS}
    ${MessageComposer_settings_HEADERS}
    ${MessageComposer_part_HEADERS}
    ${MessageComposer_followupreminder_HEADERS}
    ${MessageComposer_recipient_HEADERS}
    ${MessageComposer_imagescaling_HEADERS}
    ${MessageComposer_attachement_HEADERS}
    ${MessageComposer_job_HEADERS}

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/messagecomposer
    COMPONENT Devel
)

install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

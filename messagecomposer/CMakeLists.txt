

add_definitions(-DTRANSLATION_DOMAIN=\"libmessagecomposer\")

include_directories(
    ${CMAKE_SOURCE_DIR}/messagecomposer/recipient/
    ${CMAKE_SOURCE_DIR}/messagecomposer/job
    ${CMAKE_SOURCE_DIR}/messagecomposer/utils/
    ${CMAKE_SOURCE_DIR}/messagecomposer/composer/
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

if(BUILD_TESTING)
   add_subdirectory( tests )
   add_subdirectory( autotests )
   add_subdirectory( composer-ng/autotests )
   add_subdirectory( imagescaling/autotests )
   add_subdirectory( imagescaling/tests )
   add_subdirectory( composer-ng/tests )
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

set(messagecomposer_composerng_SRCS
     composer-ng/richtextcomposer.cpp
     composer-ng/richtextcomposercontroler.cpp
     composer-ng/richtextcomposeractions.cpp
     composer-ng/klinkdialog.cpp
     composer-ng/nestedlisthelper.cpp
     composer-ng/richtextexternalcomposer.cpp
     composer-ng/richtextcomposerimages.cpp
     composer-ng/richtextcomposersignatures.cpp
     composer-ng/richtextcomposeremailquotedecorator.cpp
     composer-ng/richtextcomposeremailquotehighlighter.cpp
     composer-ng/inserthtmldialog.cpp
)


set( messagecomposer_src
     ${messagecomposer_part_src}
     ${messagecomposer_imagescaling_src}
     ${messagecomposer_job_src}
     ${messagecomposer_composer_src}
     ${messagecomposer_recipient_src}
     ${messagecomposer_attachment_src}
     ${messagecomposer_helper_src}
     ${messagecomposer_sender_src}
     ${messagecomposer_followupreminder_SRCS}
     ${messagecomposer_composerng_SRCS}
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

add_library( messagecomposer ${messagecomposer_src} )
generate_export_header(messagecomposer BASE_NAME messagecomposer)
target_link_libraries(messagecomposer
PUBLIC
  KF5::Mime
  KF5::MessageCore
  pimcommon
  KF5::AkonadiCore
  KF5::IdentityManagement
  KF5::AkonadiMime
  kleo
PRIVATE
  KF5::MailTransport
  KF5::PimTextEdit
  KF5::TemplateParser
  KF5::MessageViewer
  KF5::AkonadiWidgets
  kdepim
  KF5::CalendarCore
  KF5::KIOCore
  Grantlee5::TextDocument
  sendlater
  followupreminder
  KF5::Archive
  KF5::Ldap
)

target_include_directories(messagecomposer PUBLIC "$<BUILD_INTERFACE:${messagecomposer_SOURCE_DIR};${messagecomposer_BINARY_DIR}>")

set_target_properties( messagecomposer PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )

install( TARGETS messagecomposer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )



add_definitions(-DTRANSLATION_DOMAIN=\"libmessagecomposer\")

include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_SOURCE_DIR}/messagecore
    ${CMAKE_SOURCE_DIR}/messageviewer
    ${CMAKE_SOURCE_DIR}/messageviewer/viewer/
    ${CMAKE_SOURCE_DIR}/messagecomposer/recipient/
    ${CMAKE_SOURCE_DIR}/messagecomposer/job
    ${CMAKE_SOURCE_DIR}/messagecomposer/utils/
    ${CMAKE_SOURCE_DIR}/messagecomposer/composer/
    ${CMAKE_SOURCE_DIR}/pimcommon/
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

#REACTIVATE
add_subdirectory( tests )
include_directories(${Grantlee_INCLUDE_DIRS})


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
)

set( messagecomposer_composer_src
     composer/composer.cpp
     composer/kmeditor.cpp
     composer/signaturecontroller.cpp
     composer/composerlineedit.cpp
     composer/composerviewbase.cpp
     composer/savecontactpreferencejob.cpp
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
     imagescaling/imagescalingjob.cpp
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

set( messagecomposer_src
     ${messagecomposer_part_src}
     ${messagecomposer_imagescaling_src}
     ${messagecomposer_job_src}
     ${messagecomposer_composer_src}
     ${messagecomposer_recipient_src}
     utils/util.cpp

     attachment/attachmentcontrollerbase.cpp
     attachment/attachmentmodel.cpp

     sender/akonadisender.cpp
     helper/messagehelper.cpp
     helper/messagefactory.cpp
     settings/messagecomposersettings.cpp
)

ki18n_wrap_ui(messagecomposer_src
     imagescaling/ui/imagescalingwidget.ui
)


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

add_library( messagecomposer ${LIBRARY_TYPE} ${messagecomposer_src} )
target_link_libraries( messagecomposer KF5::KIOCore KF5::Mime KF5::AkonadiMime KF5::AkonadiCore KF5::MailTransport KF5::PimTextEdit KF5::PimIdentities
 messagecore messageviewer templateparser kleo kdepim Grantlee::TextDocument sendlater pimcommon KF5::AkonadiWidgets)

set_target_properties( messagecomposer PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )

install( TARGETS messagecomposer ${INSTALL_TARGETS_DEFAULT_ARGS} )

add_subdirectory(imagescaling/tests)


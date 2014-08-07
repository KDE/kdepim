project(messagecore)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(autotests)

include_directories( ${Boost_INCLUDE_DIR} 
                     ${CMAKE_SOURCE_DIR}/pimcommon
                   )

add_definitions(-DTRANSLATION_DOMAIN=\"libmessagecore\")

########### next target ###############

set(messagecore_attachment_LIB_SRCS
   attachment/attachmentcollector.cpp
   attachment/attachmentcompressjob.cpp
   attachment/attachmentfromfolderjob.cpp
   attachment/attachmentfrommimecontentjob.cpp
   attachment/attachmentfromurlbasejob.cpp
   attachment/attachmentfromurljob.cpp
   attachment/attachmentloadjob.cpp
   attachment/attachmentpart.cpp
   attachment/attachmentpropertiesdialog.cpp
)

set(messagecore_misc_LIB_SRCS
   misc/imagecollector.cpp
   misc/mailinglist.cpp
   misc/mdnstateattribute.cpp
)

set(messagecore_widget_LIB_SRCS
   widgets/emailaddressrequester.cpp
   widgets/annotationdialog.cpp
)

set(messagecore_helper_LIB_SRCS
   helpers/messagehelpers.cpp
   helpers/nodehelper.cpp
)

set(messagecore_LIB_SRCS
   ${messagecore_attachment_LIB_SRCS}
   ${messagecore_misc_LIB_SRCS}
   ${messagecore_widget_LIB_SRCS}
   ${messagecore_helper_LIB_SRCS}
   settings/globalsettings.cpp
   utils/stringutil.cpp
)

kconfig_add_kcfg_files(messagecore_LIB_SRCS
  settings/globalsettings_base.kcfgc
)

ki18n_wrap_ui(messagecore_LIB_SRCS
  attachment/ui/attachmentpropertiesdialog.ui
  attachment/ui/attachmentpropertiesdialog_readonly.ui
)

add_library(messagecore ${LIBRARY_TYPE} ${messagecore_LIB_SRCS})
generate_export_header(messagecore BASE_NAME messagecore)

target_link_libraries(messagecore
  KF5::Mime
  
  KF5::KIOCore
  KF5::Abc
  KF5::PimUtils
  KF5::AkonadiCore
  KF5::PimTextEdit
  KF5::AkonadiContact
  KF5::AkonadiMime
  KF5::Archive
  pimcommon
)

target_link_libraries(messagecore LINK_INTERFACE_LIBRARIES
  KF5::Mime
  KF5::KIOCore
)

set_target_properties(messagecore PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS messagecore ${INSTALL_TARGETS_DEFAULT_ARGS})

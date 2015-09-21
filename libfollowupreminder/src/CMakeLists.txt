

add_definitions(-DTRANSLATION_DOMAIN=\"libfollowupreminder\")

set(followupreminderlib_SRCS 
    followupreminderinfo.cpp
    followupreminderutil.cpp
)

kconfig_add_kcfg_files(followupreminderlib_SRCS
    settings/followupreminderagentsettings.kcfgc
)


add_library( KF5FollowupReminder ${followupreminderlib_SRCS} )
generate_export_header(KF5FollowupReminder BASE_NAME followupreminder)
add_library(KF5::FollowupReminder ALIAS KF5FollowupReminder)

target_link_libraries(KF5FollowupReminder
PRIVATE
   KF5::ConfigGui 
   KF5::CoreAddons
   Qt5::DBus 
)

target_include_directories(KF5FollowupReminder INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/FollowupReminder/;${KDE_INSTALL_INCLUDEDIR_KF5}/followupreminder>")

target_include_directories(KF5FollowupReminder PUBLIC "$<BUILD_INTERFACE:${followupreminder_SOURCE_DIR}/src;${followupreminder_BINARY_DIR}/src>")
target_include_directories(KF5FollowupReminder PUBLIC $<TARGET_PROPERTY:KF5::AkonadiCore,INTERFACE_INCLUDE_DIRECTORIES>)


set_target_properties(KF5FollowupReminder PROPERTIES
    VERSION ${FOLLOWUPREMINDER_VERSION_STRING}
    SOVERSION ${FOLLOWUPREMINDER_SOVERSION}
    EXPORT_NAME FollowupReminder
)

install(TARGETS
    KF5FollowupReminder
    EXPORT KF5FollowupReminderTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(FollowupReminder_Camelcase_HEADERS
  HEADER_NAMES
  FollowUpReminderInfo
  FollowUpReminderUtil
  REQUIRED_HEADERS FollowupReminder_HEADERS
  PREFIX FollowupReminder
)

ecm_generate_pri_file(BASE_NAME FollowupReminder
    LIB_NAME KF5FollowupReminder
    DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/FollowupReminder
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${followupreminder_SOURCE_DIR})
install(FILES
    ${FollowupReminder_Camelcase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/FollowupReminder
    COMPONENT Devel
)

install(FILES
    ${FollowupReminder_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/followupreminder_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/followupreminderagentsettings.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/followupreminder
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

project(followupreminder)

add_definitions(-DTRANSLATION_DOMAIN=\"libfollowupreminder\")

set(followupreminderlib_SRCS 
    followupreminderinfo.cpp
    followupreminderutil.cpp
)

kconfig_add_kcfg_files(followupreminderlib_SRCS
    settings/followupreminderagentsettings.kcfgc
)


add_library( followupreminder ${followupreminderlib_SRCS} )
generate_export_header(followupreminder BASE_NAME followupreminder)
target_link_libraries( followupreminder 
PRIVATE
   KF5::ConfigGui 
   Qt5::DBus 
)
target_include_directories(followupreminder PUBLIC "$<BUILD_INTERFACE:${followupreminderagent_SOURCE_DIR};${followupreminderagent_BINARY_DIR}>")
target_include_directories(followupreminder PUBLIC $<TARGET_PROPERTY:KF5::AkonadiCore,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties( followupreminder PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )


install( TARGETS followupreminder ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

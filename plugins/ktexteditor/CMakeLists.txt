add_definitions(-DTRANSLATION_DOMAIN=\"ktexteditorkabcbridge_plugin\")

set(ktexteditorkabcbridge_PART_SRCS ktexteditorkabcbridge.cpp )

add_library(ktexteditorkabcbridge MODULE ${ktexteditorkabcbridge_PART_SRCS})

target_link_libraries(ktexteditorkabcbridge   KF5::Abc KF5::I18n
                                             KF5::IdentityManagement KF5::KCMUtils)

install(TARGETS ktexteditorkabcbridge  DESTINATION ${PLUGIN_INSTALL_DIR})

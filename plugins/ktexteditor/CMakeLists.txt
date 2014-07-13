set(ktexteditorkabcbridge_PART_SRCS ktexteditorkabcbridge.cpp )

add_library(ktexteditorkabcbridge MODULE ${ktexteditorkabcbridge_PART_SRCS})

target_link_libraries(ktexteditorkabcbridge   KF5::Abc
                                             KF5::PimIdentities KF5::KCMUtils KF5::KDELibs4Support)

install(TARGETS ktexteditorkabcbridge  DESTINATION ${PLUGIN_INSTALL_DIR})

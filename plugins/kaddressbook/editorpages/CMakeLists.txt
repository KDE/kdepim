include_directories( ${CMAKE_SOURCE_DIR}/libkleo )

add_definitions(-DTRANSLATION_DOMAIN=\"cryptopageplugin\")

########### next target ###############
set(cryptopageplugin_SRCS cryptopageplugin.cpp )

include_directories(${GPGME_INCLUDES} )

add_library(cryptopageplugin MODULE ${cryptopageplugin_SRCS})
target_link_libraries(cryptopageplugin  KF5::Contacts KF5::AkonadiContact kleo KF5::IconThemes KF5::I18n)

install(TARGETS cryptopageplugin DESTINATION ${KDE_INSTALL_LIBDIR}/akonadi/contact/editorpageplugins/)

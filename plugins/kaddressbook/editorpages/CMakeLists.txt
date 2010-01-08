include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${GPGME_INCLUDES} )

########### next target ###############

set(cryptopageplugin_SRCS cryptopageplugin.cpp )

kde4_add_plugin(cryptopageplugin ${cryptopageplugin_SRCS})
target_link_libraries(cryptopageplugin ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} kleo)

install(TARGETS cryptopageplugin DESTINATION ${DATA_INSTALL_DIR}/akonadi/contact/editorpageplugins/)

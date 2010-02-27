include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} )

########### next target ###############
if (QT_QT3SUPPORT_FOUND)
set(cryptopageplugin_SRCS cryptopageplugin.cpp )

include_directories(${GPGME_INCLUDES} )

kde4_add_plugin(cryptopageplugin ${cryptopageplugin_SRCS})
target_link_libraries(cryptopageplugin ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS} kleo)

install(TARGETS cryptopageplugin DESTINATION ${LIB_INSTALL_DIR}/akonadi/contact/editorpageplugins/)
endif (QT_QT3SUPPORT_FOUND)

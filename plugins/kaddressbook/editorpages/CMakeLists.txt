include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} )

########### next target ###############
set(cryptopageplugin_SRCS cryptopageplugin.cpp )

include_directories(${GPGME_INCLUDES} )

add_library(cryptopageplugin MODULE ${cryptopageplugin_SRCS})
target_link_libraries(cryptopageplugin  KF5::Abc kleo)

install(TARGETS cryptopageplugin DESTINATION ${LIB_INSTALL_DIR}/akonadi/contact/editorpageplugins/)

include_directories( ${GPGME_INCLUDES} )

add_subdirectory( tests ) 




########### next target ###############

set(qgpgme_LIB_SRCS eventloopinteractor.cpp dataprovider.cpp )

kde4_automoc(${qgpgme_LIB_SRCS})

kde4_add_library(qgpgme SHARED ${qgpgme_LIB_SRCS})

#Laurent add test !
target_link_libraries(qgpgme  ${KDE4_KDECORE_LIBS}  gpgme++ )

set_target_properties(qgpgme PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS qgpgme  DESTINATION ${LIB_INSTALL_DIR})

install(FILES eventloopinteractor.h dataprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/qgpgme)

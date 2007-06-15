include_directories( ${GPGME_INCLUDES} ${CMAKE_SOURCE_DIR}/gpgmepp/
	)

add_subdirectory( tests ) 




########### next target ###############

set(qgpgme_LIB_SRCS eventloopinteractor.cpp dataprovider.cpp )

kde4_automoc(${qgpgme_LIB_SRCS})

kde4_add_library(qgpgme SHARED ${qgpgme_LIB_SRCS})

#Laurent add test !
target_link_libraries(qgpgme  ${KDE4_KDECORE_LIBS}  gpgmepp )

set_target_properties(qgpgme PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS qgpgme  DESTINATION ${LIB_INSTALL_DIR})

install(FILES eventloopinteractor.h dataprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/qgpgme)

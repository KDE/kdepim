include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${KDE4_INCLUDE_DIR}/QtCrypto/
)

add_subdirectory(tests)

set(keyring_SRCS
	keyringhhrecord.cc
	keyringhhdataproxy.cc
	keyringconduit.cc
)

#set(conduit_base_KCFGS
#	recordconduitSettings.kcfgc
#)

#kde4_add_kcfg_files(keyring_SRCS ${conduit_base_KCFGS})
kde4_add_library(keyring SHARED ${keyring_SRCS})
target_link_libraries(keyring qca conduit_base ${QT_LIBRARIES})
install(TARGETS keyring LIBRARY DESTINATION ${LIB_INSTALL_DIR})

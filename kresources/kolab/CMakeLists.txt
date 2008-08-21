include_directories( ${CMAKE_SOURCE_DIR}/kmail ${CMAKE_SOURCE_DIR}/kresources/lib )
set( libresourcekolabshared_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/shared/resourcekolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kmailconnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/subresource.cpp
	${CMAKE_SOURCE_DIR}/kmail/groupware_types.cpp
	)

add_subdirectory( kabc )
add_subdirectory( knotes )
add_subdirectory( kcal )

install(FILES kolab-resource.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS upgrade-resourcetype.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})


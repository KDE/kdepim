include_directories( ${CMAKE_SOURCE_DIR}/kmail ${CMAKE_SOURCE_DIR}/kresources/lib )
set( libresourcescalixshared_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/shared/resourcescalixbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kmailconnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/subresource.cpp
	${CMAKE_SOURCE_DIR}/kmail/groupware_types.cpp
	)

add_subdirectory( kabc )
add_subdirectory( knotes )
add_subdirectory( kcal )

add_subdirectory( kioslave )
add_subdirectory( scalixadmin )

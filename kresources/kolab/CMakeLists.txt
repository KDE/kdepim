include_directories( ${CMAKE_SOURCE_DIR}/kmail  )
set( libresourcekolabshared_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/shared/resourcekolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kmailconnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/subresource.cpp
	)

add_subdirectory( kabc ) 
add_subdirectory( knotes ) 
add_subdirectory( kcal ) 


set( libresourcekolabshared_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/shared/resourcekolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kmailconnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/kolabbase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shared/subresource.cpp
	)
        #kmailconnection.skel kmailicalIface.stub

add_subdirectory( kabc ) 
add_subdirectory( knotes ) 
MESSAGE(STATUS "port kcal")
#add_subdirectory( kcal ) 


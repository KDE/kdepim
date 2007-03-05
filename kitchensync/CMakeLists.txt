project(kitchensync)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libqopensync )

ADD_SUBDIRECTORY( libqopensync ) 
ADD_SUBDIRECTORY( src ) 


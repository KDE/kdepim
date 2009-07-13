set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory( tests )

set( messagecomposer_src

     composer.cpp
     behaviour.cpp
     finalmessage.cpp
     util.cpp

     messagepart.cpp
     attachmentpart.cpp
     infopart.cpp
     skeletonmessagejob.cpp
     textpart.cpp

     job.cpp
     contentjob.cpp
     maintextjob.cpp
     multipartjob.cpp
)

include_directories(
    ${Boost_INCLUDE_DIR}
)

kde4_add_library( messagecomposer SHARED ${messagecomposer_src} )
target_link_libraries( messagecomposer ${KDE4_KIO_LIBS} kmime )
set_target_properties( messagecomposer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS messagecomposer EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES
         messagecomposer_export.h
          
         composer.h
         behaviour.h
         attachmentpart.h
         finalmessage.h
         infopart.h
         messagepart.h
         skeletonmessagejob.h
         textpart.h

         job.h
         contentjob.h
         maintextjob.h
         multipartjob.h

         DESTINATION ${INCLUDE_INSTALL_DIR}/messagecomposer COMPONENT Devel )


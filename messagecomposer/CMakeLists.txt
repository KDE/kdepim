set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory( tests )

set( messagecomposer_src

     composer.cpp
     finalmessage.cpp
     util.cpp

     messagepart.cpp
     attachmentpart.cpp
     globalpart.cpp
     infopart.cpp
     textpart.cpp

     jobbase.cpp
     contentjobbase.cpp
     singlepartjob.cpp
     multipartjob.cpp
     maintextjob.cpp

     attachmentcompressjob.cpp
     attachmentfrommimecontentjob.cpp
     attachmentfrompublickeyjob.cpp
     attachmentfromurljob.cpp
     skeletonmessagejob.cpp
)

include_directories(
    ${Boost_INCLUDE_DIR}
)

kde4_add_library( messagecomposer SHARED ${messagecomposer_src} )
target_link_libraries( messagecomposer ${KDE4_KIO_LIBS} kleo kmime kpimtextedit )
set_target_properties( messagecomposer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS messagecomposer EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES
         messagecomposer_export.h
          
         composer.h
         attachmentpart.h
         finalmessage.h
         globalpart.h
         infopart.h
         messagepart.h
         textpart.h

         jobbase.h
         contentjobbase.h
         singlepartjob.h
         multipartjob.h
         maintextjob.h

         attachmentcompressjob.h
         attachmentfrommimecontentjob.h
         attachmentfrompublickeyjob.h
         attachmentfromurljob.h
         skeletonmessagejob.h

         DESTINATION ${INCLUDE_INSTALL_DIR}/messagecomposer COMPONENT Devel )


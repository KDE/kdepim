set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory( tests )

set( messagecomposer_src

     composer.cpp
     util.cpp

     akonadisender.cpp
     messagehelper.cpp
     messagefactory.cpp
     messageinfo.cpp
     messagecomposersettings.cpp

     messagepart.cpp
     globalpart.cpp
     infopart.cpp
     textpart.cpp

     jobbase.cpp
     contentjobbase.cpp
     attachmentjob.cpp
     singlepartjob.cpp
     multipartjob.cpp
     maintextjob.cpp
     signjob.cpp
     encryptjob.cpp
     signencryptjob.cpp
     transparentjob.cpp

     skeletonmessagejob.cpp

     kmeditor.cpp
     inserttextfilejob.cpp
     signaturecontroller.cpp
     composerlineedit.cpp
     recipient.cpp
     distributionlistdialog.cpp
     recipientspicker.cpp
     recipientseditor.cpp
     kwindowpositioner.cpp
)

if(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT true)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT true)
else(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT false)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT false)
endif(KDEPIM_ENTERPRISE_BUILD)

configure_file(messagecomposer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messagecomposer.kcfg)

kde4_add_kcfg_files(messagecomposer_src
  messagecomposersettings_base.kcfgc 
)


include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkleo
)

kde4_add_library( messagecomposer SHARED ${messagecomposer_src} )
target_link_libraries( messagecomposer ${KDE4_KIO_LIBS} ${KDE4_KMIME_LIBRARY} ${KDEPIMLIBS_AKONADI_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_MAILTRANSPORT_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
 messagecore messageviewer templateparser kleo kdepim ${QGPGME_LIBRARIES} )
set_target_properties( messagecomposer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS messagecomposer ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( TARGETS messagecomposer EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES
#         messagecomposer_export.h
#         composer.h
#         globalpart.h
#         infopart.h
#         messagepart.h
#         textpart.h
#         jobbase.h
#         contentjobbase.h
#         attachmentjob.h
#         singlepartjob.h
#         multipartjob.h
#         maintextjob.h
#         signjob.h
#         encryptjob.h
#         signencryptjob.h
#         transparentjob.h
#         skeletonmessagejob.h
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagecomposer COMPONENT Devel )


set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
    ${Boost_INCLUDE_DIR}
    ${SOPRANO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_SOURCE_DIR}/messagecore
    ${CMAKE_SOURCE_DIR}/messageviewer
    ${CMAKE_SOURCE_DIR}/messagecomposer/recipient/
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory( tests )
include_directories(${Grantlee_INCLUDE_DIRS})

set( messagecomposer_src

     composer.cpp
     util.cpp

     attachmentcontrollerbase.cpp
     attachmentmodel.cpp
     attachmentfrompublickeyjob.cpp

     akonadisender.cpp
     messagehelper.cpp
     messagefactory.cpp
     messagecomposersettings.cpp
     composerviewbase.cpp

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

     recipient/recipientspicker.cpp
     recipient/recipient.cpp
     recipient/recipientline.cpp
     recipient/recipientseditor.cpp
     recipient/recipientseditorsidewidget.cpp
     recipient/kwindowpositioner.cpp

     aliasesexpandjob.cpp
     emailaddressresolvejob.cpp

     keyresolver.cpp
     autocorrection/composerautocorrection.cpp
     autocorrection/composerautocorrectionwidget.cpp
     autocorrection/composerautocorrectionlistwidget.cpp
     autocorrection/composerautocorrectiontreewidget.cpp
     autocorrection/composerautocorrectionlanguage.cpp
     autocorrection/import/importlibreofficeautocorrection.cpp
     autocorrection/import/importkmailautocorrection.cpp
     autocorrection/import/importabstractautocorrection.cpp
     imagescaling/imagescalingjob.cpp
     imagescaling/imagescalingwidget.cpp
     imagescaling/imagescalingutils.cpp
     imagescaling/imagescalingselectformat.cpp
)

kde4_add_ui_files(messagecomposer_src
     autocorrection/composerautocorrectionwidget.ui
     imagescaling/ui/imagescalingwidget.ui
)


if (NOT WINCE)
  # disabled due to no QTreeWidget
  set( messagecomposer_src ${messagecomposer_src}
     distributionlistdialog.cpp
  )
endif ()

if(KDEPIM_ENTERPRISE_BUILD)
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT true)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT true)
else()
  set(WARN_TOOMANY_RECIPIENTS_DEFAULT false)
  set(ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT false)
endif()

configure_file(messagecomposer.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/messagecomposer.kcfg)

kde4_add_kcfg_files(messagecomposer_src
  messagecomposersettings_base.kcfgc 
)

kde4_add_library( messagecomposer ${LIBRARY_TYPE} ${messagecomposer_src} )
target_link_libraries( messagecomposer ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_MAILTRANSPORT_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
 messagecore messageviewer templateparser kleo kdepim ${QGPGME_LIBRARIES} )

target_link_libraries( messagecomposer ${Grantlee_GUI_LIBRARIES})

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


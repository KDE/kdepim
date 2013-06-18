set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_subdirectory(tests)

include_directories( ${Boost_INCLUDE_DIR} 
                     ${SOPRANO_INCLUDE_DIR}
		     ${NEPOMUK_CORE_INCLUDE_DIR}
                   )

########### next target ###############

configure_file(ontology/messagetag.ontology.in ${CMAKE_CURRENT_BINARY_DIR}/messagetag.ontology)

install(FILES  ontology/messagetag.trig ${CMAKE_CURRENT_BINARY_DIR}/messagetag.ontology DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ontology/kde)



set(messagecore_LIB_SRCS

   nepomukutil/asyncnepomukresourceretriever.cpp
   misc/imagecollector.cpp
   attachment/attachmentcollector.cpp
   attachment/attachmentcompressjob.cpp
   attachment/attachmentfromfolderjob.cpp
   attachment/attachmentfrommimecontentjob.cpp
   attachment/attachmentfromurlbasejob.cpp
   attachment/attachmentfromurljob.cpp
   attachment/attachmentloadjob.cpp
   attachment/attachmentpart.cpp
   attachment/attachmentpropertiesdialog.cpp
   widgets/emailaddressrequester.cpp
   settings/globalsettings.cpp
   misc/mailinglist.cpp
   misc/mdnstateattribute.cpp
   helpers/messagehelpers.cpp
   helpers/nodehelper.cpp
   utils/stringutil.cpp
)

if (NOT WINCE)
  set(messagecore_LIB_SRCS
     ${messagecore_LIB_SRCS}
     widgets/annotationdialog.cpp
  )
endif ()

kde4_add_kcfg_files(messagecore_LIB_SRCS
  settings/globalsettings_base.kcfgc
)

kde4_add_ui_files(messagecore_LIB_SRCS
  attachment/attachmentpropertiesdialog.ui
  attachment/attachmentpropertiesdialog_readonly.ui
)

kde4_add_library(messagecore ${LIBRARY_TYPE} ${messagecore_LIB_SRCS})

target_link_libraries(messagecore
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
)

target_link_libraries(messagecore LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(messagecore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS messagecore ${INSTALL_TARGETS_DEFAULT_ARGS})

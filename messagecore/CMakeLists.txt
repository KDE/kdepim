set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_subdirectory(tests)

include_directories( ${Boost_INCLUDE_DIR} )

########### next target ###############

set(messagecore_LIB_SRCS
   attachmentpart.cpp
   attachmentcompressjob.cpp
   attachmentloadjob.cpp
   attachmentfromurljob.cpp
   attachmentfrommimecontentjob.cpp
   attachmentcollector.cpp
   attachmentpropertiesdialog.cpp
   messagestatus.cpp
   annotationdialog.cpp
   stringutil.cpp
   nodehelper.cpp
   mailinglist-magic.cpp
   kmfawidgets.cpp
)

kde4_add_ui_files(messagecore_LIB_SRCS
  attachmentpropertiesdialog.ui
)

kde4_add_library(messagecore SHARED ${messagecore_LIB_SRCS})

target_link_libraries(messagecore
  ${KDEPIMLIBS_KMIME_LIBS} 
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_LIBRARIES}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
)

target_link_libraries(messagecore LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(messagecore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS messagecore ${INSTALL_TARGETS_DEFAULT_ARGS})

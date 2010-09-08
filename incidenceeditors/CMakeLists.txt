if (KDEPIM_MOBILE_UI)
  remove_definitions(-DKDEPIM_MOBILE_UI)
endif()
#
include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditors
    ${CMAKE_BINARY_DIR}/incidenceeditors
    ${CMAKE_SOURCE_DIR}/kdgantt1
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
    ${CMAKE_SOURCE_DIR}/calendarsupport
    ${CMAKE_BINARY_DIR}/calendarsupport
    ${Boost_INCLUDE_DIR}
)
#
#add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
#if (QT_QT3SUPPORT_FOUND)
#  add_definitions( -DHAVE_QT3SUPPORT )
#endif ()
#
########### next target ###############
#
#if (NOT WINCE)
#
#set( incidenceeditors_LIB_SRCS
#    autochecktreewidget.cpp
#    categoryselectdialog.cpp
#    designerfields.cpp
#    editoralarms.cpp
#    editorattachments.cpp
#    editorconfig.cpp
#    editorgeneral.cpp
#    editorgeneraljournal.cpp
#    embeddedurlpage.cpp
#    freebusyurldialog.cpp
#    incidenceeditor.cpp
#    journaleditor.cpp
#    templatemanagementdialog.cpp
#)
#
#if (QT_QT3SUPPORT_FOUND)
#  set( incidenceeditors_LIB_SRCS
#    ${incidenceeditors_LIB_SRCS}
#    ${libkdgantt1_SRCS}
#    attendeeeditor.cpp
#    editordetails.cpp
#    editorfreebusy.cpp
# )
#endif ()
#
#kde4_add_ui_files( incidenceeditors_LIB_SRCS
#   categoryselectdialog_base.ui
#   editoralarms_base.ui
#   template_management_dialog_base.ui
#)
#
#kde4_add_library(incidenceeditors ${LIBRARY_TYPE} ${incidenceeditors_LIB_SRCS})
#
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
#
#target_link_libraries(incidenceeditors
#   ${ZLIB_LIBRARY}
#   ${QT_QTUITOOLS_LIBRARY}
#   ${KDEPIMLIBS_KCALCORE_LIBS}
#   ${KDEPIMLIBS_KCALUTILS_LIBS}
#   ${KDE4_KPARTS_LIBRARY}
#   kdepim
#   kdepimdbusinterfaces
#  calendarsupport
#   incidenceeditorsng
#)
#
#if (QT_QT3SUPPORT_FOUND)
#  target_link_libraries(incidenceeditors ${KDE4_KDE3SUPPORT_LIBRARY})
#endif ()
#
#set_target_properties(incidenceeditors
#                      PROPERTIES VERSION ${GENERIC_LIB_VERSION}
#                      SOVERSION ${GENERIC_LIB_SOVERSION})
#install(TARGETS incidenceeditors ${INSTALL_TARGETS_DEFAULT_ARGS})

#endif (NOT WINCE)

add_subdirectory( incidenceeditor-ng )

include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditors
    ${CMAKE_BINARY_DIR}/incidenceeditors
    ${CMAKE_SOURCE_DIR}/kdgantt1
    ${CMAKE_SOURCE_DIR}/libkdepim
    ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
    ${CMAKE_SOURCE_DIR}/akonadi/kcal
    ${CMAKE_BINARY_DIR}/akonadi/kcal
    ${Boost_INCLUDE_DIR}
)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
if (QT_QT3SUPPORT_FOUND)
  add_definitions( -DHAVE_QT3SUPPORT )
endif ()

########### next target ###############

set( incidenceeditors_LIB_SRCS
    incidenceeditor-ng/incidenceeditor.cpp
    incidenceeditor-ng/combinedincidenceeditor.cpp
    incidenceeditor-ng/incidencegeneraleditor.cpp
    incidenceeditor-ng/incidenceeditorgeneralpage.cpp

    #incidencegeneraleditor.cpp
    autochecktreewidget.cpp
    categoryconfig.cpp
    categoryhierarchyreader.cpp
    categoryselectdialog.cpp
    designerfields.cpp
    editoralarms.cpp
    editorattachments.cpp
    editorconfig.cpp
    editorgeneral.cpp
    editorgeneralevent.cpp
    editorgeneraljournal.cpp
    editorgeneraltodo.cpp
    editorrecurrence.cpp
    embeddedurlpage.cpp
    eventeditor.cpp
    freebusyurldialog.cpp
    groupwareintegration.cpp
    incidenceeditor.cpp
    journaleditor.cpp
    templatemanagementdialog.cpp
    todoeditor.cpp
)

if (QT_QT3SUPPORT_FOUND)
  set( incidenceeditors_LIB_SRCS
    ${incidenceeditors_LIB_SRCS}
    ${libkdgantt1_SRCS}
    attendeeeditor.cpp
    editordetails.cpp
    editorfreebusy.cpp
  )
endif ()

kde4_add_ui_files( incidenceeditors_LIB_SRCS
   categoryselectdialog_base.ui
   editoralarms_base.ui
   #incidencegeneral.ui
   template_management_dialog_base.ui
   incidenceeditor-ng/incidencegeneraleditor.ui
)

kde4_add_library(incidenceeditors SHARED ${incidenceeditors_LIB_SRCS})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

target_link_libraries(incidenceeditors
   ${ZLIB_LIBRARY}
   ${QT_QT3SUPPORT_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${QT_QTUITOOLS_LIBRARY}
   ${KDE4_KDEUI_LIBS}
   ${KDE4_KPARTS_LIBS}
   ${KDE4_KMIME_LIBRARY}
   ${KDEPIMLIBS_KCAL_LIBS}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
   kdepim
   kdepimdbusinterfaces
   akonadi-kcal_next
)

if (QT_QT3SUPPORT_FOUND)
  target_link_libraries(incidenceeditors ${KDE4_KDE3SUPPORT_LIBRARY})
endif ()

set_target_properties(incidenceeditors PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS incidenceeditors ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory( tests )

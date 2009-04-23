
include_directories( ${CMAKE_SOURCE_DIR}/kmail ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/mimelib ${KDE4_INCLUDE_DIR} )


########### next target ###############

set(kmail_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp )


kde4_add_plugin(kmail_bodypartformatter_text_vcard ${kmail_bodypartformatter_text_vcard_PART_SRCS})



kdepim4_link_unique_libraries(kmail_bodypartformatter_text_vcard  ${KDE4_KDECORE_LIBS} ${KDE4_KMIME_LIBS} ${KDE4_KABC_LIBS} kdepim)

install(TARGETS kmail_bodypartformatter_text_vcard  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kmail_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp delegateselector.cpp attendeeselector.cpp)

qt4_add_dbus_interfaces(kmail_bodypartformatter_text_calendar_PART_SRCS
  ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml
  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml
)

kde4_add_ui_files(kmail_bodypartformatter_text_calendar_PART_SRCS ui_attendeeselector.ui)

kde4_add_plugin(kmail_bodypartformatter_text_calendar ${kmail_bodypartformatter_text_calendar_PART_SRCS})



kdepim4_link_unique_libraries(kmail_bodypartformatter_text_calendar  ${KDE4_KPIMUTILS_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS} kmailprivate  kdepim ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kmail_bodypartformatter_text_calendar  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kmail_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp )


kde4_add_plugin(kmail_bodypartformatter_text_xdiff ${kmail_bodypartformatter_text_xdiff_PART_SRCS})



kdepim4_link_unique_libraries(kmail_bodypartformatter_text_xdiff  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} )

install(TARGETS kmail_bodypartformatter_text_xdiff  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES text_vcard.desktop text_calendar.desktop text_xdiff.desktop  DESTINATION ${DATA_INSTALL_DIR}/kmail/plugins/bodypartformatter)

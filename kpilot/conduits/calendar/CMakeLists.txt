include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../akonadibase
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../akonadibase
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
        ${Boost_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set( calendar_SRCS
	calendarakonadirecord.cc
	calendarakonadiproxy.cc
	calendarconduit.cc
	calendarconfig.cc
	calendarhhdataproxy.cc
	calendarhhrecord.cc
	calendarfactory.cc
)

kde4_add_ui_files( calendar_SRCS settingswidget.ui )
kde4_add_kcfg_files( calendar_SRCS calendarsettings.kcfgc )
kde4_add_library( kpilot_conduit_calendar2 SHARED ${calendar_SRCS} )

target_link_libraries( kpilot_conduit_calendar2
	${KDE4_KCAL_LIBS} ${KDE4_AKONADI_LIBS}
	kpilot_akonadibase kpilot_conduit_base kpilot
)

set_target_properties( kpilot_conduit_calendar2 PROPERTIES PREFIX "" )

install( TARGETS kpilot_conduit_calendar2 LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kpilot-conduit-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES calendarsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})

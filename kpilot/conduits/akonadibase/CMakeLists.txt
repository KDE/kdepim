find_package(Boost)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIRS}
)

message(STATUS "Boost includes ${Boost_INCLUDE_DIRS}")

set(akonadibase_SRCS
	akonadirecord.cc
	akonadidataproxy.cc
	akonadisetupwidget.cc
)

kde4_add_ui_files(akonadibase_SRCS akonadi-setup-widget.ui)

kde4_add_library(kpilot_akonadibase SHARED ${akonadibase_SRCS})

target_link_libraries(kpilot_akonadibase
	kpilot_conduit_base kpilot
	${KDE4_KDEUI_LIBS}
	${KDEPIMLIBS_AKONADI_LIBS}
)

install(TARGETS kpilot_akonadibase LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS})

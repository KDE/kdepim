
include_directories( 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces 
	${CMAKE_SOURCE_DIR}/kaddressbook/
	${KDE4_INCLUDE_DIR} 
	${QT_INCLUDES} 
	)

kde4_add_kcfg_files(libkabcommon_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )

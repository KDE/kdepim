kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabcfrontend_SRCS main.cpp )

kde4_automoc(${kabcfrontend_SRCS})

kde4_add_ui_files(kabcfrontend_SRCS mainWindow.ui )

kde4_add_executable(kabcfrontend ${kabcfrontend_SRCS})

target_link_libraries(kabcfrontend  ${QT_AND_KDECORE_LIBS} kabc )

install_targets(/bin kabcfrontend )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(includedir)/kabc $(all_includes)
#
#METASOURCES = AUTO
#
#bin_PROGRAMS = kabcfrontend
#
#kabcfrontend_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kabcfrontend_LDADD = -lkabc $(LIB_KDECORE)
#kabcfrontend_SOURCES = main.cpp mainWindow.ui
#





########### next target ###############

set(kabcfrontend_SRCS main.cpp )

kde4_automoc(${kabcfrontend_SRCS})

kde4_add_ui_files(kabcfrontend_SRCS mainWindow.ui )

kde4_add_executable(kabcfrontend ${kabcfrontend_SRCS})

target_link_libraries(kabcfrontend  ${KDE4_KDECORE_LIBS} kabc )

install(TARGETS kabcfrontend  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





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

kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kconf_update FILES korn-3-4-config_change.upd korn-3-5-update.upd )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=$(all_includes)
#
#update_DATA = korn-3-4-config_change.upd korn-3-5-update.upd
#update_SCRIPTS = korn-3-5-ssl-update.pl korn-3-5-metadata-update.pl
#updatedir = $(kde_datadir)/kconf_update
#
#kconf_PROGRAMS = korn-3-4-config_change
#kconfdir = $(libdir)/kconf_update_bin
#
#korn_3_4_config_change_SOURCES = korn-3-4-config_change.cpp
#korn_3_4_config_change_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#korn_3_4_config_change_LDADD = $(LIB_QT)

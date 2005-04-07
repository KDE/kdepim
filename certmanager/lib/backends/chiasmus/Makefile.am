INCLUDES = -I$(top_srcdir)/certmanager/lib -I$(top_srcdir)/libkdenetwork $(all_includes)

KDE_CXXFLAGS = '-DGPG_ERR_SOURCE_DEFAULT=((gpg_err_source_t)176)'

noinst_LTLIBRARIES = libkleopatra_backend_chiasmus.la

libkleopatra_backend_chiasmus_la_SOURCES = \
                        config_data.c \
                        obtainkeysjob.cpp \
                        chiasmusjob.cpp \
                        chiasmuslibrary.cpp \
                        chiasmusbackend.cpp


METASOURCES = AUTO

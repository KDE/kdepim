SUBDIRS = versit tests . calendarimap

INCLUDES = -I$(top_srcdir)/libical/src/libical \
  -I$(top_srcdir)/libical/src/libicalss -I$(top_builddir)/libical/src/libical \
  -I$(top_builddir)/libical/src/libicalss -I$(srcdir)/versit $(all_includes)

lib_LTLIBRARIES   = libkcal.la
libkcal_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:0 -module
libkcal_la_LIBADD  = $(LIB_QT) versit/libversit.la $(LIB_KFILE) \
                     $(top_builddir)/libical/src/libical/libical.la \
                     $(top_builddir)/libical/src/libicalss/libicalss.la \
                     -lkderesources

libkcal_la_SOURCES = \
        incidencebase.cpp incidence.cpp journal.cpp todo.cpp event.cpp \
        freebusy.cpp attendee.cpp attachment.cpp recurrence.cpp alarm.cpp \
        customproperties.cpp calendar.cpp calendarlocal.cpp \
        calformat.cpp vcalformat.cpp icalformat.cpp icalformatimpl.cpp \
	vcaldrag.cpp icaldrag.cpp \
        exceptions.cpp \
        scheduler.cpp imipscheduler.cpp dummyscheduler.cpp \
        calfilter.cpp \
        person.cpp period.cpp duration.cpp \
        dndfactory.cpp \
        filestorage.cpp \
        compat.cpp \
	resourcecalendar.cpp resourcelocal.cpp resourcelocalconfig.cpp \
	calendarresources.cpp \
        qtopiaformat.cpp

kde_module_LTLIBRARIES = libkcm_kcalendars.la kcal_dir.la kcal_remote.la kcal_kabc.la

libkcm_kcalendars_la_SOURCES = kcmcalendars.cpp
libkcm_kcalendars_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kcalendars_la_LIBADD = libkcal.la -lkderesources

kcal_dir_la_SOURCES = resourcelocaldir.cpp resourcelocaldirconfig.cpp
kcal_dir_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_dir_la_LIBADD  = libkcal.la -lkderesources

kcal_remote_la_SOURCES = resourceremote.cpp resourceremoteconfig.cpp
kcal_remote_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_remote_la_LIBADD  = libkcal.la -lkderesources

kcal_kabc_la_SOURCES = resourcekabc.cpp resourcekabcconfig.cpp
kcal_kabc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_kabc_la_LIBADD  = libkcal.la -lkderesources -lkabc

EXTRA_PROGRAMS = convertqtopia
convertqtopia_SOURCES = convertqtopia.cpp
convertqtopia_LDFLAGS = $(all_libraries)
convertqtopia_LDADD = libkcal.la

datadir = $(kde_appsdir)/Settings/Components
data_DATA = kcalendars.desktop

linkdir= $(kde_datadir)/kresources/calendar
link_DATA = local.desktop localdir.desktop remote.desktop kabc.desktop

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkcal.pot

include ../admin/Doxyfile.am

## Makefile.am for kandy

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = .

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kandy kandy_client

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# the library search path. 
kandy_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
kandy_LDADD   = $(LIB_KFILE) $(LIB_KAB)

# which sources should be compiled for kandy
kandy_SOURCES =	main.cpp kandy.cpp \
                cmdpropertiesdialog_base.ui cmdpropertiesdialog.cpp \
                kandyview.cpp \
		kandyiface.skel \
                modem.cpp kandyview_base.ui \
                atcommand.cpp commanditem.cpp mobilegui_base.ui mobilegui.cpp \
                commandscheduler.cpp commandset.cpp \
                kprefs.cpp kprefsdialog.cpp kandyprefs.cpp kandyprefsdialog.cpp \
                mobilemain.cpp

# these are the headers for your project
noinst_HEADERS   = kandy.h kandyview.h modem.h atcommand.h \
                   commanditem.h mobilegui.h commandscheduler.h

# client stuff
kandy_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kandy_client_LDADD   = $(LIB_KDECORE)
kandy_client_SOURCES = kandy_client.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kandy.pot

KDE_ICON = kandy

# this is where the kdelnk file will go 
kdelnkdir   = $(kde_appsdir)/Utilities
kdelnk_DATA = kandy.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kandy
rc_DATA = kandyui.rc kandymobileui.rc

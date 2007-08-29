bin_PROGRAMS = scalixadmin

INCLUDES = $(all_includes)

scalixadmin_LDFLAGS = $(KDE_RPATH) $(all_libraries)
scalixadmin_LDADD   = $(LIB_KABC)
scalixadmin_SOURCES = main.cpp mainwindow.cpp passwordpage.cpp settings.cpp jobs.cpp \
                      otherusermanager.cpp otheruserview.cpp otheruserpage.cpp ldapview.cpp ldapdialog.cpp \
                      delegatemanager.cpp delegateview.cpp delegatepage.cpp delegatedialog.cpp \
                      outofofficepage.cpp

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/scalixadmin.pot

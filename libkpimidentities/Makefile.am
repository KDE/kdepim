INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/certmanager/lib $(all_includes)

lib_LTLIBRARIES = libkpimidentities.la
libkpimidentities_la_SOURCES = \
    identitymanager.cpp identity.cpp identitymanager.skel \
    identitycombo.cpp
libkpimidentities_la_LIBADD = ../certmanager/lib/libkleopatra.la \
    ../libkdepim/libkdepim.la ../libemailfunctions/libemailfunctions.la
libkpimidentities_la_LDFLAGS = $(all_libraries) -no-undefined \
    -version-info 1:0:0

identitymanager_DCOPIDLNG = true

METASOURCES = AUTO

# No messages target. Those files are part of libkdepim.pot.

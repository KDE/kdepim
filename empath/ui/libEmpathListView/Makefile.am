KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libEmpathListView.la

libEmpathListView_la_LDFLAGS = $(all_libraries) \
  -version-info 1:0:0 -module -no-undefined

libEmpathListView_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE)

METASOURCES = AUTO

libEmpathListView_la_SOURCES = EmpathListView.cpp

INCLUDES= $(all_includes) -I ../

bin_PROGRAMS = test

test_SOURCES = test.cpp

test_LDADD = -lkabui $(LIB_KDEUI) $(LIB_KDECORE)

test_LDFLAGS = -L../ $(all_libraries) $(KDE_RPATH)


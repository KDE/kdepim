INCLUDES = $(all_includes)

bin_PROGRAMS = calendar3

calendar3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
calendar3_LDADD   = $(LIB_KDEUI)
calendar3_SOURCES = calendar3.cpp calhijri.cpp mkdatepicker.cpp \
                    kcalendarsystem.cpp \
                    mkdatetbl.cpp hconv.c

METASOURCES = AUTO

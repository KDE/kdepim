INCLUDES                = $(all_includes)
lib_LTLIBRARIES         = libkimap.la
libkimap_la_SOURCES      = \
AsyncClient.cpp \
Client.cpp \
ListResponse.cpp \
MailboxInfo.cpp \
Response.cpp \
StatusInfo.cpp \
Utils.cpp

libkimap_la_LDFLAGS      = -module -version-info 0:0 -no-undefined
libkimap_la_LIBADD       = $(all_libraries) $(LIB_QT)
libkimap_la_METASOURCES  = AUTO

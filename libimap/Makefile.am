INCLUDES                = $(all_includes)
lib_LTLIBRARIES         = libimap.la
libimap_la_SOURCES      = \
AsyncClient.cpp \
Client.cpp \
ListResponse.cpp \
MailboxInfo.cpp \
Response.cpp \
StatusInfo.cpp \
Utils.cpp

libimap_la_LDFLAGS      = -module -version-info 0:0 -no-undefined
libimap_la_LIBADD       = $(all_libraries) $(LIB_QT)
libimap_la_METASOURCES  = AUTO

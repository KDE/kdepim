SUBDIRS           = rmm
INCLUDES          = -Irmm $(all_includes)
lib_LTLIBRARIES   = librmm.la
librmm_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
librmm_la_LIBADD  = $(LIB_QT)

librmm_la_SOURCES = \
Address.cpp \
AddressList.cpp \
BodyPart.cpp \
ContentDisposition.cpp \
ContentType.cpp \
Cte.cpp \
DateTime.cpp \
Enum.cpp \
Envelope.cpp \
Header.cpp \
Mailbox.cpp \
Mechanism.cpp \
MessageID.cpp \
MimeType.cpp \
Parameter.cpp \
ParameterList.cpp \
Text.cpp \
Token.cpp \
Utilities.cpp



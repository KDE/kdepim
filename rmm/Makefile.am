INCLUDES=$(all_includes)

noinst_PROGRAMS = test
test_SOURCES = test.cpp
test_LDFLAGS = $(all_libraries)
test_LDADD = -L$(top_builddir)/rmm -lrmm

lib_LTLIBRARIES = librmm.la
librmm_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
# Define this for testing.
# -no-undefined 

librmm_la_LIBADD = $(LIB_QT)

librmm_la_SOURCES = \
RAddress.cpp \
RAddressList.cpp \
RBodyPart.cpp \
RContentType.cpp \
RCte.cpp \
RDateTime.cpp \
RDispositionType.cpp \
REnum.cpp \
REnvelope.cpp \
RHeader.cpp \
RHeaderBody.cpp \
RMailbox.cpp \
RMechanism.cpp \
RMessage.cpp \
RMessageComponent.cpp \
RMessageID.cpp \
RMimeType.cpp \
RParameter.cpp \
RParameterList.cpp \
RText.cpp \
RToken.cpp \
RUtilities.cpp

include_HEADERS = \
RMM_Address.h \
RMM_AddressList.h \
RMM_AddressList_generated.h \
RMM_Address_generated.h \
RMM_BodyPart.h \
RMM_BodyPart_generated.h \
RMM_ContentType.h \
RMM_ContentType_generated.h \
RMM_Cte.h \
RMM_Cte_generated.h \
RMM_DateTime.h \
RMM_DateTime_generated.h \
RMM_Defines.h \
RMM_DispositionType.h \
RMM_DispositionType_generated.h \
RMM_Entity.h \
RMM_Enum.h \
RMM_Envelope.h \
RMM_Envelope_generated.h \
RMM_Header.h \
RMM_HeaderBody.h \
RMM_HeaderBody_generated.h \
RMM_HeaderList.h \
RMM_Header_generated.h \
RMM_Mailbox.h \
RMM_Mailbox_generated.h \
RMM_Mechanism.h \
RMM_Mechanism_generated.h \
RMM_Message.h \
RMM_MessageComponent.h \
RMM_MessageID.h \
RMM_MessageID_generated.h \
RMM_Message_generated.h \
RMM_MimeType.h \
RMM_MimeType_generated.h \
RMM_Parameter.h \
RMM_ParameterList.h \
RMM_ParameterList_generated.h \
RMM_Parameter_generated.h \
RMM_Text.h \
RMM_Text_generated.h \
RMM_Token.h \
RMM_Utility.h


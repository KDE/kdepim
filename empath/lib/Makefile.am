KDE_CXXFLAGS = $(USE_RTTI)

INCLUDES = -I$(top_srcdir)/rmm/ $(all_includes)

lib_LTLIBRARIES = libempathkernel.la

libempathkernel_la_LDFLAGS = $(all_libraries)
libempathkernel_la_LIBADD = $(LIB_KDECORE)

METASOURCES = AUTO

libempathkernel_la_SOURCES = \
Empath.cpp \
EmpathViewFactory.cpp \
EmpathInterface.cpp \
EmpathJobInfo.cpp \
EmpathCachedMessage.cpp \
EmpathComposeForm.cpp \
EmpathComposer.cpp \
EmpathEditorProcess.cpp \
EmpathFilter.cpp \
EmpathFilterEventHandler.cpp \
EmpathFilterList.cpp \
EmpathFolder.cpp \
RDatabase.cpp \
EmpathIndex.cpp \
EmpathIndexRecord.cpp \
EmpathMailSender.cpp \
EmpathMailSenderQmail.cpp \
EmpathMailSenderSMTP.cpp \
EmpathMailSenderSendmail.cpp \
EmpathMailbox.cpp \
EmpathMailboxIMAP4.cpp \
EmpathMailboxList.cpp \
EmpathMailboxMaildir.cpp \
EmpathMailboxPOP3.cpp \
EmpathMaildir.cpp \
EmpathMatcher.cpp \
EmpathQuotedText.cpp \
EmpathSecurityProcess.cpp \
EmpathTask.cpp \
EmpathURL.cpp \
EmpathUtilities.cpp

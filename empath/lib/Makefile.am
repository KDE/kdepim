KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST

INCLUDES = -I$(top_srcdir)/librmm/ $(all_includes)

lib_LTLIBRARIES = libempathkernel.la

libempathkernel_la_LDFLAGS = $(all_libraries) $(LIBQPTHREAD) $(LIBPTHREAD) \
	-version-info 1:0:0 -module 
	
include_HEADERS = EmpathInterface.h

libempathkernel_la_LIBADD = $(LIB_KDECORE) \
	$(top_builddir)/librmm/librmm.la

METASOURCES = AUTO

libempathkernel_la_SOURCES = \
EmpathInterface.skel \
Empath.cpp \
EmpathJob.cpp \
EmpathJobScheduler.cpp \
EmpathComposeForm.cpp \
EmpathComposer.cpp \
EmpathCustomEvents.cpp \
EmpathFilter.cpp \
EmpathFilterEventHandler.cpp \
EmpathFilterList.cpp \
EmpathFolder.cpp \
EmpathIndex.cpp \
EmpathMailSender.cpp \
EmpathMailSenderQmail.cpp \
EmpathMailSenderSMTP.cpp \
EmpathMailSenderSendmail.cpp \
EmpathMailbox.cpp \
EmpathMailboxList.cpp \
EmpathMailboxMaildir.cpp \
EmpathMaildir.cpp \
EmpathMailboxPOP3.cpp \
IMAPClient.cpp \
EmpathMailboxIMAP4.cpp \
EmpathMatcher.cpp \
EmpathQuotedText.cpp \
EmpathSecurityProcess.cpp \
EmpathTask.cpp \
EmpathTaskTimer.cpp \
EmpathURL.cpp \
EmpathUtilities.cpp

KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST

INCLUDES = -I$(top_srcdir)/rmm/ $(all_includes)

lib_LTLIBRARIES = libempathkernel.la

libempathkernel_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
#	Define this for testing.
#	-no-undefined

libempathkernel_la_LIBADD = $(LIB_KDECORE) \
	$(top_builddir)/rmm/librmm.la

METASOURCES = AUTO

libempathkernel_la_SOURCES = \
Empath.cpp \
EmpathInterface.cpp \
EmpathJob.cpp \
EmpathJobScheduler.cpp \
EmpathCachedMessage.cpp \
EmpathComposeForm.cpp \
EmpathComposer.cpp \
EmpathEditorProcess.cpp \
EmpathFilter.cpp \
EmpathFilterEventHandler.cpp \
EmpathFilterList.cpp \
EmpathFolder.cpp \
EmpathIndex.cpp \
EmpathIndexRecord.cpp \
EmpathMailSender.cpp \
EmpathMailSenderQmail.cpp \
EmpathMailSenderSMTP.cpp \
EmpathMailSenderSendmail.cpp \
EmpathMailbox.cpp \
EmpathMailboxList.cpp \
EmpathMailboxMaildir.cpp \
EmpathMaildir.cpp \
EmpathMailboxPOP3.cpp \
EmpathMatcher.cpp \
EmpathQuotedText.cpp \
EmpathTask.cpp \
EmpathTaskTimer.cpp \
EmpathURL.cpp \
EmpathUtilities.cpp \
EmpathViewFactory.cpp \
EmpathSecurityProcess.cpp

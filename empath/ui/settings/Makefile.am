KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = $(all_includes) -I$(top_srcdir)/empath/lib

lib_LTLIBRARIES = libEmpathSettingsDialog.la

libEmpathSettingsDialog_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module

libEmpathSettingsDialog_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE)

METASOURCES = AUTO

libEmpathSettingsDialog_la_SOURCES = \
EmpathAccountsSettingsDialog.cpp \
EmpathConfigIMAP4Dialog.cpp \
EmpathConfigMaildirDialog.cpp \
EmpathConfigPOP3Dialog.cpp \
EmpathConfigPOP3Logging.cpp \
EmpathConfigPOP3Server.cpp \
EmpathConfigPOP3Widget.cpp \
EmpathDialogButtonBox.cpp \
EmpathDisplaySettingsDialog.cpp \
EmpathFilterActionDialog.cpp \
EmpathFilterEditDialog.cpp \
EmpathFilterListItem.cpp \
EmpathFilterManagerDialog.cpp \
EmpathIdentitySettingsDialog.cpp \
EmpathMatchPropertiesDialog.cpp \
EmpathSendingSettingsDialog.cpp \
EmpathSettingsDialog.cpp

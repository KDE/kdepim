INCLUDES = -I$(top_srcdir)/kab2/lib -I$(top_srcdir)/kab2/server $(all_includes)

bin_PROGRAMS = kab_client kab_definition_editor

kab_client_SOURCES = kab_client.cpp

kab_client_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(top_builddir)/kab2/server/libKAddressBookServer.la \
  $(LIB_KDECORE)

kab_definition_editor_SOURCES = \
  DefinitionEntryEditorBase.ui \
  DefinitionEditorBase.ui \
  DefinitionEntryEditor.cpp \
  DefinitionEditor.cpp \
  kab_definition_editor.cpp

kab_definition_editor_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(LIB_KDEUI)

kab_definition_editor_METASOURCES = AUTO

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkab2.la

libkab2_la_METASOURCES = USE_AUTOMOC

AM_CXXFLAGS = -rdynamic
libkab2_la_LIBADD = -ldl
libkab2_la_SOURCES = \
Kab2.cpp \
AddressBook.cpp \
AddressBookClient.cpp \
Entity.cpp \
Field.cpp \
Group.cpp

doc:
	kdoc -d ../doc/srcdoc/ -n kab2 -i -e --strip-h-path -H \
Kab2.h \
KabAddressBook.h \
KabAddressBookClient.h \
KabEntity.h \
KabField.h \
KabGroup.h

dox:
	doxygen libkab2.doxygen


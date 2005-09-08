TEMPLATE = app

TARGET = ../bin/kabcclient

CONFIG += qt thread debug warn_on

SOURCES = main.cpp \
          csvtemplate.cpp \
          csvtemplatefactory.cpp \
          formatfactory.cpp \
          inputformatimpls.cpp \
          kabcclient.cpp \
          outputformatimpls.cpp

HEADERS = csvtemplate.h \
          csvtemplatefactory.h \
          formatfactory.h \
          inputformat.h \
          inputformatimpls.h \
          kabcclient.h \
          outputformat.h \
          outputformatimpls.h

KDEINCLUDES = /usr/include/kde
INCLUDEPATH += $$KDEINCLUDES

KDELIBS = /usr
LIBS += -L $$KDELIBS -lkabc -lkdecore

MOC_DIR = .mocs
OBJECTS_DIR = .objs
#-------------------------------------------------
#
# Project created by QtCreator 2010-04-02T01:13:24
#
#-------------------------------------------------

TARGET = mail_grantlee
TEMPLATE = app


SOURCES += main.cpp\
        mailwindow.cpp

HEADERS  += mailwindow.h

RESOURCES += \
    mail_grantlee.qrc

# install
target.path = mail_grantlee
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.html *.css images
INSTALLS += target sources

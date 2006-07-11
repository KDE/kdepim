# Fil skapad av Kdevelops qmake-hanterare. 
# ------------------------------------------- 
# Delkatalog relativ till projektets huvudkatalog: .
# Målet är ett program: 

HEADERS += conversationdelegate.h \
           dummykonadiadapter.h \
           conversation.h \
           message.h \
           foldermodel.h \
           mailview.h \
           conversationview.h 
SOURCES += conversationdelegate.cpp \
           dummykonadiadapter.cpp \
           conversation.cpp \
           message.cpp \
           foldermodel.cpp \
           mailview.cpp \
           main.cpp \
           conversationview.cpp 
TEMPLATE = app
TARGET +=
DEPENDPATH += .
INCLUDEPATH += .

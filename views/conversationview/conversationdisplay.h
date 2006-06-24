#ifndef CONVERSATIONDISPLAY_H
#define CONVERSATIONDISPLAY_H


#include "dummykonadiadapter.h"

class ConversationDisplay : public QTextEdit
{
  Q_OBJECT
public:
  conversationDisplay(const DummyKonadiAdapter &dummydata, QObject *parent = 0) : QTextEdit(parent), backend(dummydata) {}
	~conversationDisplay();
	
public slot:
	void setConversation(QModelIndex &index);
	
private:
  DummyKonadiAdapter* backend;
};

#endif

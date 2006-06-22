/*
 * dummykonadiconversation.h
 * DummyKonadiConversation -- a dummy mockup of what a conversation should look like in my adapter for aKonadi
 * Oh, and GPL, LGPL, BSD, QPL or whatever OSI approved license you prefer
 */

#ifndef DUMMYKONADICONVERSATION_H
#define DUMMYKONADICONVERSATION_H

#include <QString>
#include <QList>

#include "dummykonadimessage.h"

class DummyKonadiConversation
{
public:
  DummyKonadiConversation(QString *conversationTitle = 0) : title(*conversationTitle) {}
  int count() const;
  QString conversationTitle() const;
  DummyKonadiMessage message(int messageId) const;
  void addMessage(DummyKonadiMessage &message);
  QString author(int messageId) const;
  QString content(int messageId) const;

//public signals:
//  void messageAdded();

private:
  QString title;
  QList<DummyKonadiMessage> messages;
};

#endif

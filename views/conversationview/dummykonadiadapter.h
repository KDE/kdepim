/*
 * dummykonadiadapter.h
 * DummyKonadiAdapter -- a dummy mockup of what I need of aKonadi, sort of...
 * Oh, and GPL, LGPL, BSD, QPL or whatever OSI approved license you prefer
 */

#ifndef DUMMYKONADIADAPTER_H
#define DUMMYKONADIADAPTER_H

#include <QString>
#include <QStringList>

#include "dummykonadiconversation.h"

class DummyKonadiAdapter
{
public:
  DummyKonadiAdapter();
  ~DummyKonadiAdapter();
  int conversationCount() const;
  QString conversationTitle(int conversationId) const;
  QString messageContent(int conversationId, int messageId) const;
  QString messageAuthor(int conversationId, int messageId) const;
  int messageCount(int conversationId) const;

private:
  bool tryConversationId(int conversationId) const;
  QList<DummyKonadiConversation*> conversations;
};

#endif

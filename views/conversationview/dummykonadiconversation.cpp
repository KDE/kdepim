#include "dummykonadiconversation.h"

int DummyKonadiConversation::count() const
{
  return messages.count();
}

QString DummyKonadiConversation::conversationTitle() const
{
  return title;
}

DummyKonadiMessage DummyKonadiConversation::message(int messageId) const
{
  if (messageId < 0 || messageId >= messages.count())
    return DummyKonadiMessage(true);
  return messages.at(messageId);
}

void DummyKonadiConversation::addMessage(DummyKonadiMessage &message)
{
  messages << message;
//  emit messageAdded();
}

QString DummyKonadiConversation::author(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.author();
  return 0;
}

QString DummyKonadiConversation::content(int messageId) const
{
  DummyKonadiMessage tmp = message(messageId);
  if (! tmp.isNull())
    return tmp.content();
  return 0;
}

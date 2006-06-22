#include "dummykonadimessage.h"

QString DummyKonadiMessage::author() const
{
  if (nullContent == true) return "";
  return conversationAuthor;
}

QString DummyKonadiMessage::content() const
{
  if (nullContent == true) return "";
  return conversationContent;
}

void DummyKonadiMessage::setAuthor(QString author)
{
  conversationAuthor = author;
}

void DummyKonadiMessage::setContent(QString content)
{
  conversationContent = content;
}

bool DummyKonadiMessage::isNull()
{
  return nullContent;
}

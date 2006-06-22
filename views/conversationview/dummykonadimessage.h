/*
 * dummykonadimessage.h
 * DummyKonadiMessage -- a dummy mockup of what a message should look like in my adapter for aKonadi
 * Oh, and GPL, LGPL, BSD, QPL or whatever OSI approved license you prefer
 * @author Aron Boström <aron bostrom gmail com>
 */

#ifndef DUMMYKONADIMESSAGE_H
#define DUMMYKONADIMESSAGE_H

#include <QString>
#include <QObject>

class DummyKonadiMessage
{
public:
  DummyKonadiMessage(bool null = false) : nullContent(null) {}
  QString author() const;
  QString content() const;
  void setAuthor(QString newAuthor);
  void setContent(QString newContent);
  bool isNull();

private:
  QString conversationAuthor, conversationContent;
  bool nullContent;
};

#endif

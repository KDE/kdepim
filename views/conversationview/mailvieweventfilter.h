//
// C++ Interface: mailvieweventfilter
//
// Description: 
//
//
// Author: Aron Boström <aron dot bostrom at gmail dot com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAILVIEWEVENTFILTER_H
#define MAILVIEWEVENTFILTER_H

#include <QObject>

/**
  @author Aron Boström <aron dot bostrom at gmail dot com>
*/
class MailViewEventFilter : public QObject
{
  Q_OBJECT
public:
  MailViewEventFilter(QWidget *textedit, QWidget *viewport, QObject *parent = 0);

  ~MailViewEventFilter();

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  QWidget *m_viewport, *m_textedit;
};

#endif

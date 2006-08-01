//
// C++ Implementation: mailvieweventfilter
//
// Description: 
//
//
// Author: Aron Boström <aron dot bostrom at gmail dot com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtDebug>

#include <QResizeEvent>
#include <QWidget>

#include "mailvieweventfilter.h"

MailViewEventFilter::MailViewEventFilter(QWidget *textedit, QWidget *viewport, QObject *parent) : QObject(parent), m_viewport(viewport), m_textedit(textedit)
{
}


MailViewEventFilter::~MailViewEventFilter()
{
}

bool MailViewEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::Resize) {
    QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
    m_viewport->resize(resizeEvent->size());
    m_textedit->resize(resizeEvent->size());
    qDebug() << "TEST";
    return true;
  }
  return QObject::eventFilter(obj, event);
}

#include "mailvieweventfilter.moc"

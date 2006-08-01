//
// C++ Implementation: maildisplay
//
// Description: 
//
//
// Author: Aron Boström <aron dot bostrom at gmail dot com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "maildisplay.h"
#include "mailvieweventfilter.h"

MailDisplay::MailDisplay(QWidget *parent) : QTextEdit(parent)
{
  setHtml("sdf sdfkjdhfkjshf");
  //setReadOnly(true); 
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  installEventFilter(new MailViewEventFilter(this, viewport(), this));
//  m_edit1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

MailDisplay::~MailDisplay()
{
}

#include "maildisplay.moc"

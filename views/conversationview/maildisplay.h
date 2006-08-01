//
// C++ Interface: maildisplay
//
// Description: 
//
//
// Author: Aron Boström <aron dot bostrom at gmail dot com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAILDISPLAY_H
#define MAILDISPLAY_H

#include <QTextEdit>

/**
	@author Aron Boström <aron dot bostrom at gmail dot com>
*/
class MailDisplay : public QTextEdit
{
  Q_OBJECT
public:
  MailDisplay(QWidget *parent = 0);

  ~MailDisplay();

};

#endif

/* logWindow.h                  KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _LOGWINDOW_H
#define _LOGWINDOW_H

#include <kdialogbase.h>

class QLabel;
class QMultiLineEdit;
class KProgress;
class Config;

class LogWindow : public KDialogBase
{
Q_OBJECT
public:
	typedef enum { Progress=1 , Message=2 , Log=4 } Parts;

	LogWindow(QWidget *parent=0,int show=Progress | Message | Log);
	LogWindow(QWidget *parent=0,Config *c=0);

	virtual void showEvent(QShowEvent *);

public slots:
	void setValue(int i);
	void setMessage(const QString &s);
	void appendLog(const QString &s);

	void saveYourself(Config *);

	void configChanged(Config *);

signals:
	void shown();

protected:
	void createParts();
	void showParts(int);

private:
	KProgress *fProgress;
	QLabel *fText;
	QMultiLineEdit *fLog;

	bool fAddTime;
} ;

#endif

// $Log:$

#ifndef _POPMAIL_SETUPDIALOG_H
#define _POPMAIL_SETUPDIALOG_H
/* setupDialog.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "plugin.h"

class PopMailWidget; // From setup-dialog.ui

class PopMailWidgetConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	PopMailWidgetConfig(QWidget *, const char *);
	virtual void load();
	virtual void commit();

	static ConduitConfigBase *create(QWidget *w, const char *n)
		{ return new PopMailWidgetConfig(w,n); } ;

	// These enums must follow the order of items in the combo box
	enum SendMode { NoSend=0, SendKMail=1 } ;

protected:
	PopMailWidget *fConfigWidget;

public slots:
	void toggleSendMode(int);
} ;


#endif

#ifndef _KPILOT_VCAL_SETUP_H
#define _KPILOT_VCAL_SETUP_H
/* vcal-setup.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** It also defines the class for the behavior of the setup dialog.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"

class VCalWidget;

class VCalWidgetSetup : public ConduitConfig
{
Q_OBJECT
public:
	VCalWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~VCalWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	VCalWidget *fConfigWidget;
} ;

#endif

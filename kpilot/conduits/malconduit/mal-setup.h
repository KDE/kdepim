#ifndef _MAL_SETUP_H
#define _MAL_SETUP_H
/* mal-setup.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the widget and behavior for the config dialog
** of the mal conduit.
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

class MALWidget;

class MALWidgetSetup : public ConduitConfig
{
Q_OBJECT
public:
	MALWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~MALWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	MALWidget *fConfigWidget;
} ;

// $Log$

#endif

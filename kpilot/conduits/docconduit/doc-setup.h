#ifndef _DOC_DOC_SETUP_H
#define _DOC_DOC_SETUP_H
/* doc-setup.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the widget and behavior for the config dialog
** of the doc conduit.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"

class DOCWidget;

class DOCWidgetConfig : public ConduitConfigBase
{
public:
	DOCWidgetConfig(QWidget *, const char *);
	virtual void commit(KConfig *);
	virtual void load(KConfig *);
protected:
	DOCWidget *fConfigWidget;
} ;

class DOCWidgetSetup : public ConduitConfig
{
public:
	DOCWidgetSetup(QWidget *, const char *, const QStringList &);
	virtual ~ DOCWidgetSetup();

	virtual void readSettings();

 protected:
	virtual void commitChanges();

 private:
	DOCWidgetConfig * fConfigBase;
};

#endif

#ifndef _KPILOT_KNOTES_SETUP_H
#define _KPILOT_KNOTES_SETUP_H
/* knotes-setup.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the widget and behavior for the config dialog
** of the KNotes conduit.
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

class KNotesWidget;

class KNotesConfigBase : public ConduitConfigBase
{
public:
	KNotesConfigBase(QWidget *parent, const char *name);
	
	virtual void commit(KConfig *);
	virtual void load(KConfig *);

private:
	KNotesWidget *fConfigWidget;
} ;

class KNotesWidgetSetup : public ConduitConfig
{
public:
	KNotesWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~KNotesWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	KNotesConfigBase *fConfigBase;
} ;

#endif

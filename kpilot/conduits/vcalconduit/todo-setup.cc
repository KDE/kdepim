/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
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

#include "options.h"

#include <qbuttongroup.h>

#include "korganizerConduit.h"
#include "todo-factory.h"
#include "todo-setup.h"



ToDoWidgetSetup::ToDoWidgetSetup(QWidget *w, const char *n) :
	VCalWidgetSetupBase(w,n)
{
	FUNCTIONSETUP;
	fConduitName = i18n("To-do");
	UIDialog::addAboutPage(fConfigWidget->tabWidget,VCalConduitFactoryBase::about());

	fConfigWidget->fSyncDestination->setTitle(i18n("To-do Destination"));
}

ToDoWidgetSetup::~ToDoWidgetSetup()
{
	FUNCTIONSETUP;
}

/* static */ ConduitConfigBase *ToDoWidgetSetup::create(QWidget *w, const char *n)
{
	return new ToDoWidgetSetup(w,n);
}

VCalConduitSettings*ToDoWidgetSetup::config() 
{
  return ToDoConduitFactory::config(); 
}


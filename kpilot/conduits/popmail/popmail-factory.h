#ifndef _KPILOT_POPMAIL_FACTORY_H
#define _KPILOT_POPMAIL_FACTORY_H
/* popmail-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the popmail-conduit plugin.
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

#include <klibloader.h>

#include "plugin.h"

class KInstance;
class KAboutData;

class PopMailSendPage;
class PopMailReceivePage;

#if 0
class PopmailWidgetSetup : public ConduitConfig
{
// Q_OBJECT
public:
	PopmailWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~PopmailWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	PopMailSendPage *fSendPage;
	PopMailReceivePage *fRecvPage;
} ;
#endif

class PopMailConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	PopMailConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~PopMailConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	static const char *group() { return fGroup; } ;
	static const char *syncIncoming() { return fSyncIncoming; } ;
	static const char *syncOutgoing() { return fSyncOutgoing; } ;

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
	
	// Config keys for the mail conduit.
	//
	//
	static const char * const fGroup;
	static const char * const fSyncOutgoing,
		* const fSyncIncoming;
} ;

extern "C"
{

void *init_conduit_popmail();

} ;

#endif

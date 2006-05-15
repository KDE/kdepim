#ifndef _KPILOT_CONFIGWIZARD_H
#define _KPILOT_CONFIGWIZARD_H
/* kpilotConfigWizard.h                 KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines kpilot's configuration wizard
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

#include "kwizard.h"

class ConfigWizard_base1;
class ConfigWizard_base2;
class ConfigWizard_base3;

class ConfigWizard : public KWizard
{
Q_OBJECT
public:
	enum Mode { InDialog=0, Standalone=1 } ;

	ConfigWizard(QWidget *p=0L,const char *n=0L, int mode=(int)InDialog);
	~ConfigWizard();

protected slots:
	void probeHandheld();
protected:
	void accept();
//	ConfigWizard_base1 *page1;
	ConfigWizard_base2 *page2;
	ConfigWizard_base3 *page3;

	Mode fMode;
	QStringList mDBs;
} ;

#endif

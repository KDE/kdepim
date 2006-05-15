/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the doc-conduit plugin.
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
#include "doc-factory.moc"
#include "doc-factory.h"

#include <kinstance.h>
#include <kaboutdata.h>
#include <kpilotlink.h>

#include "doc-conduit.h"
#include "doc-setup.h"


extern "C" {
	void *init_conduit_doc() {
		return new DOCConduitFactory;
	}
}



// A number of static variables
KAboutData * DOCConduitFactory::fAbout = 0L;

const char *DOCConduitFactory::dbDOCtype = "TEXt";
const char *DOCConduitFactory::dbDOCcreator = "REAd";



DOCConduitFactory::DOCConduitFactory(QObject * p, const char *n):
KLibFactory(p, n)
{
	FUNCTIONSETUP;
	fInstance = new KInstance("docconduit");
	fAbout =new KAboutData("docconduit",
		I18N_NOOP("Palm DOC Conduit for KPilot"), KPILOT_VERSION,
		I18N_NOOP("Configures the DOC Conduit for KPilot"),
		KAboutData::License_GPL, "(C) 2002, Reinhold Kainhofer");

	fAbout->addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Maintainer"), "reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com");
}

DOCConduitFactory::~DOCConduitFactory()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}


/* virtual */ QObject * DOCConduitFactory::createObject(QObject * p,
	const char *n, const char *c, const QStringList & a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname <<": Creating object of class " <<c <<endl;
#endif
	if (qstrcmp(c, "ConduitConfigBase") == 0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);
		if (w)
		{
			return new DOCWidgetConfig(w,n);
		}
		else
		{
			kdError() << k_funcinfo 
				<< ": Couldn't cast parent to widget." << endl;
			return 0L;
		}
	}
	if (qstrcmp(c, "SyncAction") == 0)
	{
		KPilotDeviceLink * d = dynamic_cast < KPilotDeviceLink * >(p);
		if (d)
		{
			return new DOCConduit(d, n, a);
		}
		else
		{
			kdError() << k_funcinfo
				<<": Couldn't cast parent to KPilotDeviceLink" <<endl;
			return 0L;
		}
	}
	return 0L;
}


/* kpalmDOCConverter.cpp
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This is the main program of the KDE PalmDOC converter.
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


static const char *kpalmdoc_id =
	"$Id$";


#include "options.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>

#include <converterdlg_base.h>



int main(int argc, char **argv)
{

	KAboutData about("kpalmdoc", I18N_NOOP("KPalmDOC"),
		"-0.0.1",
		"KPalmDOC - KDE Converter for PalmDOC texts.\n\n",
		KAboutData::License_GPL, "(c) 2003, Reinhold Kainhofer");
	about.addAuthor("Reinhold Kainhofer",
		I18N_NOOP("Main Developer"),
		"reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer of KPilot"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");

	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;
	PalmDOCDialog *dlg=new PalmDOCDialog();
	dlg->show();
	return app.exec();
	
	/* NOTREACHED */
	(void) kpalmdoc_id;
}


/* dbViewerWidget.cc		KPilot
**
** Copyright (C) 2003 by Dan Pilone.
**	Authored by Adriaan de Groot
**
** This is the generic DB viewer widget.
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


#include "config.h"
#include "options.h"

#include <pi-file.h>

#include <qlayout.h>
#include <qdir.h>

#include <klistbox.h>
#include <ktextedit.h>

#include "listCat.h"

#include "dbviewerWidget.h"

GenericDBWidget::GenericDBWidget(QWidget *parent, const QString &dbpath) :
	PilotComponent(parent,"component_generic",dbpath)
{
	FUNCTIONSETUP;

	QGridLayout *g = new QGridLayout(this,3,5,SPACING);
	fList = new KListBox(this);
	g->addWidget(fList,1,1);
	fDisplay = new KTextEdit(this);
	fDisplay->setReadOnly(true);
	g->addWidget(fDisplay,1,3);
	
	connect(fList,SIGNAL(highlighted(const QString &)),
		this,SLOT(slotSelected(const QString &)));
}

GenericDBWidget::~GenericDBWidget()
{
	FUNCTIONSETUP;
}


void GenericDBWidget::initialize()
{
	FUNCTIONSETUP;
	
	fList->clear();
	fDisplay->setText(QString::null);

	QDir dir(dbPath());
	dir.setNameFilter(CSL1("*.pdb;*.prc"));
	QStringList l = dir.entryList();

	for (QStringList::ConstIterator i = l.begin();
		i != l.end();
		++i)
	{
		fList->insertItem(*i);
	}
	
	
	fList->show();
	fDisplay->show();
}

void GenericDBWidget::slotSelected(const QString &dbname)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Selected DB " << dbname << endl;
#endif

	QCString filename = QFile::encodeName(dbPath() + CSL1("/") + dbname);
	const char *s = filename;
	struct pi_file *pf = pi_file_open(const_cast<char *>(s));
	if (!pf)
	{
		fDisplay->setText(i18n("<B>Warning:</B> Cannot read "
			"database file %1.").arg(dbname));
		return;
	}
	
	struct DBInfo dbinfo;
	if (pi_file_get_info(pf,&dbinfo))
	{
		fDisplay->setText(i18n("<B>Warning:</B> Cannot read "
			"database file %1.").arg(dbname));
		return;
	}
	
	QString display(i18n("<B>Database:</B> %1<BR><BR>").arg(dbname));
	
	QDateTime ttime;
	
	ttime.setTime_t(dbinfo.createDate);
	display.append(i18n("Created: %1<BR>").arg(ttime.toString()));
	
	ttime.setTime_t(dbinfo.modifyDate);
	display.append(i18n("Modified: %1<BR>").arg(ttime.toString()));
	
	ttime.setTime_t(dbinfo.backupDate);
	display.append(i18n("Backed up: %1<BR>").arg(ttime.toString()));
	
	fDisplay->setText(display);
	pi_file_close(pf);
}


#include "dbviewerWidget.moc"

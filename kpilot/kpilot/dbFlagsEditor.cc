/* KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
**
**/

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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <pi-dlp.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdatewidget.h>
#include <kmessagebox.h>

#include "pilotRecord.h"
#include "dbFlagsEditor.h"
#include "dbFlagsEditor_base.h"


DBFlagsEditor::DBFlagsEditor(DBInfo*dbinfo, QWidget *parent) :
	KDialog(parent)
	,dbi(dbinfo)
{
	setButtons(Ok|Cancel);
	setCaption(i18n("Edit Database Flags"));
	setDefaultButton(Ok);
	widget=new DBFlagsEditorWidget(this);
	setMainWidget(widget);
	fillWidgets();
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
	connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}


DBFlagsEditor::~DBFlagsEditor()
{
}

void DBFlagsEditor::slotOk()
{
	if (KMessageBox::questionYesNo(this, i18n("Changing the database flags might corrupt the whole database, or make the data unusable. Do not change the values unless you are absolutely sure you know what you are doing.\n\nReally assign these new flags?"), i18n("Changing Database Flags"),KGuiItem(i18n("Assign")),KStandardGuiItem::cancel())==KMessageBox::Yes)
	{
		Pilot::toPilot(widget->fDBName->text(),dbi->name,33);

		char buff[5];
		QString type = widget->fType->text();
		buff[0]=type[0].unicode();
		buff[1]=type[1].unicode();
		buff[2]=type[2].unicode();
		buff[3]=type[3].unicode();
		buff[4]=0;
		dbi->type=get_long(buff);

		type = widget->fCreator->text();
		buff[0]=type[0].unicode();
		buff[1]=type[1].unicode();
		buff[2]=type[2].unicode();
		buff[3]=type[3].unicode();
		buff[4]=0;
		dbi->creator=get_long(buff);


#define setflag(ctrl, flag) if (widget->ctrl->isChecked()) dbi->flags |=flag;\
	else dbi->flags &= ~flag;

		setflag(fRessourceDB, dlpDBFlagResource);
		setflag(fReadOnly, dlpDBFlagReadOnly);
		setflag(fBackupDB, dlpDBFlagBackup);
		setflag(fCopyProtect, dlpDBFlagCopyPrevention);
		setflag(fReset, dlpDBFlagReset);
#undef setflag

		if (widget->fExcludeDB->isChecked())
			dbi->miscFlags |= dlpDBMiscFlagExcludeFromSync;
		else	dbi->miscFlags &= ~dlpDBMiscFlagExcludeFromSync;

		QDateTime ttime;
		ttime.setDate(widget->fCreationDate->date());
		ttime.setTime(widget->fCreationTime->time());
		dbi->createDate=ttime.toTime_t();

		ttime.setDate(widget->fModificationDate->date());
		ttime.setTime(widget->fModificationTime->time());
		dbi->modifyDate=ttime.toTime_t();

		ttime.setDate(widget->fBackupDate->date());
		ttime.setTime(widget->fBackupTime->time());
		dbi->backupDate=ttime.toTime_t();

		accept();
	}
}

void DBFlagsEditor::slotCancel()
{
	reject();
}

void DBFlagsEditor::fillWidgets()
{
	// FUNCTIONSETUP

	widget->fDBName->setText(QString::fromLatin1(dbi->name));

	char buff[5];
	set_long(buff, dbi->type);
	buff[4]='\0';
	widget->fType->setText(QString::fromLatin1(buff));
	set_long(buff, dbi->creator);
	buff[4]='\0';
	widget->fCreator->setText(QString::fromLatin1(buff));

	widget->fRessourceDB->setChecked(dbi->flags & dlpDBFlagResource);
	widget->fReadOnly->setChecked(dbi->flags & dlpDBFlagReadOnly);
	widget->fBackupDB->setChecked(dbi->flags & dlpDBFlagBackup);
	widget->fCopyProtect->setChecked(dbi->flags & dlpDBFlagCopyPrevention);

	widget->fReset->setChecked(dbi->flags & dlpDBFlagReset);
	widget->fExcludeDB->setChecked(dbi->miscFlags & dlpDBMiscFlagExcludeFromSync);

	QDateTime ttime;
	ttime.setTime_t(dbi->createDate);
	widget->fCreationDate->setDate(ttime.date());
	widget->fCreationTime->setTime(ttime.time());

	ttime.setTime_t(dbi->modifyDate);
	widget->fModificationDate->setDate(ttime.date());
	widget->fModificationTime->setTime(ttime.time());
	ttime.setTime_t(dbi->backupDate);
	widget->fBackupDate->setDate(ttime.date());
	widget->fBackupTime->setTime(ttime.time());
}


#include "dbFlagsEditor.moc"

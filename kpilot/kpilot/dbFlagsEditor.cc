/* dbFlagsEditor.cc                KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Writeen 2003 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <pi-dlp.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdatewidget.h>
#if KDE_IS_VERSION(3,1,90)
#include <ktimewidget.h>
#endif
#include <kmessagebox.h>

#include "dbFlagsEditor.h"
#include "dbFlagsEditor_base.h"


DBFlagsEditor::DBFlagsEditor(DBInfo*dbinfo, QWidget *parent) :
	KDialogBase(parent, "FlagsEditor",false,
		i18n("Edit Database Flags"), Ok|Cancel), 
	dbi(dbinfo)
{
	widget=new DBFlagsEditorWidget(this);
	setMainWidget(widget);
	fillWidgets();
}


DBFlagsEditor::~DBFlagsEditor()
{
}

void DBFlagsEditor::slotOk()
{
	if (KMessageBox::questionYesNo(this, i18n("Changing the database flags might corrupt the whole database, or make the data unusable. Do not change the values unless you are absolutely sure you know what you are doing.\n\nReally assign these new flags?"), i18n("Changing Database Flags"))==KMessageBox::Yes)
	{
		strlcpy(dbi->name, widget->fDBName->text().latin1(), 33);

		char buff[5];
		strlcpy(buff, widget->fType->text().latin1(), 5);
		dbi->type=get_long(buff);

		strlcpy(buff, widget->fCreator->text().latin1(), 5);
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
#if KDE_IS_VERSION(3,1,9)
		ttime.setTime(widget->fCreationTime->time());
#endif
		dbi->createDate=ttime.toTime_t();

		ttime.setDate(widget->fModificationDate->date());
#if KDE_IS_VERSION(3,1,9)
		ttime.setTime(widget->fModificationTime->time());
#endif
		dbi->modifyDate=ttime.toTime_t();

		ttime.setDate(widget->fBackupDate->date());
#if KDE_IS_VERSION(3,1,9)
		ttime.setTime(widget->fBackupTime->time());
#endif
		dbi->backupDate=ttime.toTime_t();

		KDialogBase::slotOk();
	}
}

void DBFlagsEditor::slotCancel()
{
	KDialogBase::slotCancel();
}

void DBFlagsEditor::fillWidgets()
{
	// FUNCTIONSETUP

	widget->fDBName->setText(dbi->name);

	char buff[5];
	set_long(buff, dbi->type);
	buff[4]='\0';
	widget->fType->setText(buff);
	set_long(buff, dbi->creator);
	buff[4]='\0';
	widget->fCreator->setText(buff);

	widget->fRessourceDB->setChecked(dbi->flags & dlpDBFlagResource);
	widget->fReadOnly->setChecked(dbi->flags & dlpDBFlagReadOnly);
	widget->fBackupDB->setChecked(dbi->flags & dlpDBFlagBackup);
	widget->fCopyProtect->setChecked(dbi->flags & dlpDBFlagCopyPrevention);

	widget->fReset->setChecked(dbi->flags & dlpDBFlagReset);
	widget->fExcludeDB->setChecked(dbi->miscFlags & dlpDBMiscFlagExcludeFromSync);

	QDateTime ttime;
	ttime.setTime_t(dbi->createDate);
	widget->fCreationDate->setDate(ttime.date());
#if KDE_IS_VERSION(3,1,9)
	widget->fCreationTime->setTime(ttime.time());
#endif

	ttime.setTime_t(dbi->modifyDate);
	widget->fModificationDate->setDate(ttime.date());
#if KDE_IS_VERSION(3,1,9)
	widget->fModificationTime->setTime(ttime.time());
#endif

	ttime.setTime_t(dbi->backupDate);
	widget->fBackupDate->setDate(ttime.date());
#if KDE_IS_VERSION(3,1,9)
	widget->fBackupTime->setTime(ttime.time());
#endif
}


#include "dbFlagsEditor.moc"

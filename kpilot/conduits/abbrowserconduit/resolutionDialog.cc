/* resolutionDialog.h			KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
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

#include <qtimer.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qregexp.h>

#include "resolutionTable.h"
#include "resolutionDialog_base.h"

#include "resolutionDialog.moc"

/** This class describes the controllers of the conflict resolution ListView,
 *  as well as its child radio buttons. There are two different constructors
 *  for them.
 *  Each controller has three child radio buttons, and if any of them is
 *  activated (stateChange), it sets the text of its parent (which is the
 *  controller, which is an instance of ResolutionCheckListItem, too).
 **/
class ResolutionCheckListItem : QCheckListItem {
public:
	ResolutionCheckListItem(ResolutionItem*it, ResolutionTable*tb,
		QListView*parent);
	ResolutionCheckListItem(QString header, QString text,
		ResolutionCheckListItem*parent);
	~ResolutionCheckListItem() {};
	virtual void stateChange(bool newstate);
	virtual void setValue(QString text);
	virtual void setCaption(QString caption);

protected:
	void updateText();
	/* fResItem is only set for the controller */
	ResolutionItem*fResItem;
	bool isController;
	/* The description of the entry, e.g. Backup, PC, Palm for the radio buttons,
	 * of the field name for the controllers
	 */
	QString fCaption;
	/* The currrent value of the entry (for controllers this changes with the
	 * selected button */
	QString fText;
};


ResolutionCheckListItem::ResolutionCheckListItem(ResolutionItem*it,
		ResolutionTable*tb, QListView*parent) :
	QCheckListItem(parent, QString::null, QCheckListItem::Controller),
	fResItem(it),
	isController(true),
	fCaption(it?(it->fName):(QString::null)),
	fText(it?(it->fResolved):(QString::null))
{
	FUNCTIONSETUP;
	if (it && tb)
	{
		// If all three texts are identical, there is no need for
		// resolution so don't show the radio items below
		bool itemsEqual=true;
		QString testtext(QString::null);
		const enum eExistItems its[3]={eExistsPC, eExistsPalm, eExistsBackup};
		// get a valid text from a valid field, which will serve as the
		// test text for the comparison
		for (int i=0; i<3; i++)
		{
			if ((testtext.isNull()) && (it->fExistItems & its[i]) )
				testtext=it->fEntries[i];
		}
		for (int i=0; i<3; i++)
		{
			if (it->fExistItems & its[i])
				itemsEqual&=(it->fEntries[i]==testtext);
		}
		if (!itemsEqual)
		{
			ResolutionCheckListItem*item;
			for (int i=2; i>=0; i--)
			{
				// Add only existing items
				if (it->fExistItems & its[i])
				{
					item=new ResolutionCheckListItem(it->fEntries[i], tb->labels[i], this);
					item->setOn(it->fEntries[i]==fText);
				}
			}
		}
		updateText();
	}
	setOpen(true);
}

ResolutionCheckListItem::ResolutionCheckListItem(QString text, QString header,
		ResolutionCheckListItem*parent) :
	QCheckListItem(parent, QString(), QCheckListItem::RadioButton),
	fResItem(0L),
	isController(false),
	fCaption(header),
	fText(text)
{
	updateText();
}

void ResolutionCheckListItem::stateChange(bool newstate)
{
	if (newstate && !isController)
	{
		ResolutionCheckListItem*par=static_cast<ResolutionCheckListItem*>(parent());
		{
			par->setValue(fText);
		}
	}
}

void ResolutionCheckListItem::setValue(QString text)
{
	FUNCTIONSETUP;
	fText=text;
	if (isController && fResItem)
	{
		fResItem->fResolved=text;
	}
	updateText();
}

void ResolutionCheckListItem::setCaption(QString caption)
{
	fCaption=caption;
	updateText();
}

void ResolutionCheckListItem::updateText()
{
	QString newText(i18n("Entries in the resolution dialog. First the name of the field, then the entry from the Handheld or PC after the colon", "%1: %2").arg(fCaption).arg(fText));
	newText.replace(QRegExp(CSL1("\n")),
		i18n("Denoting newlines in Address entries. No need to translate", " | "));
	setText(0, newText);
}



/*****************************************************************
 *
 *****************************************************************/

ResolutionDlg::ResolutionDlg( QWidget* parent, KPilotDeviceLink*fH,
	QString caption, QString helpText, ResolutionTable*tab) :
	KDialogBase( parent, "ResolutionDlg", false, caption, Apply|Cancel, Apply),
	tickleTimer(0L),
	fHandle(fH),
	fTable(tab)
{
	fWidget = new ResolutionDialogBase( this );
	setMainWidget(fWidget);
	fTable->fResolution=SyncAction::eDoNothing;
	fWidget->fIntroText->setText(helpText);

	fillListView();
	adjustButtons(tab);

	adjustSize();
	resize(size());

	if (fHandle) tickleTimer=new QTimer(this, "TickleTimer");

	if (tickleTimer)
	{
		connect( tickleTimer, SIGNAL(timeout()), this, SLOT(_tickle()));
		// tickle the palm every 10 seconds to prevent a timeout until the
		// sync is really finished.
		tickleTimer->start( 10000 );
	}

	connect(fWidget->fKeepBoth, SIGNAL(clicked()), SLOT(slotKeepBoth()));
	connect(fWidget->fBackupValues, SIGNAL(clicked()), SLOT(slotUseBackup()));
	connect(fWidget->fPalmValues, SIGNAL(clicked()), SLOT(slotUsePalm()));
	connect(fWidget->fPCValues, SIGNAL(clicked()), SLOT(slotUsePC()));
}

void ResolutionDlg::adjustButtons(ResolutionTable*tab)
{
	FUNCTIONSETUP;
	if (!tab) return;
	if (!(tab->fExistItems & eExistsPC) )
	{
		fWidget->fPCValues->setText(i18n("Delete entry"));
		fWidget->fKeepBoth->setDisabled(TRUE);
		fWidget->fKeepBoth->hide();
	}
	if (!(tab->fExistItems & eExistsPalm) )
	{
		fWidget->fPalmValues->setText(i18n("Delete entry"));
		fWidget->fKeepBoth->setDisabled(TRUE);
		fWidget->fKeepBoth->hide();
	}
	if (!(tab->fExistItems & eExistsBackup) )
	{
		fWidget->fBackupValues->setDisabled(TRUE);
	}
}

void ResolutionDlg::fillListView()
{
	FUNCTIONSETUP;
	fWidget->fResolutionView->setSorting(-1, FALSE);
	fWidget->fResolutionView->clear();
	for ( ResolutionItem* it = fTable->last(); it; it = fTable->prev() )
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Building table, items="<<it->fExistItems<<", PC="<<
			it->fEntries[0]<<", Palm="<<it->fEntries[1]<<", Backup="<<
			it->fEntries[2]<<endl;
#endif
		bool hasValidValues=false;
		if (it->fExistItems & eExistsPC)
			hasValidValues = hasValidValues || !(it->fEntries[0].isEmpty());
		if (it->fExistItems & eExistsPalm)
			hasValidValues = hasValidValues || !(it->fEntries[1].isEmpty());
		if (it->fExistItems & eExistsBackup)
			hasValidValues = hasValidValues || !(it->fEntries[2].isEmpty());
		if (hasValidValues)
			new ResolutionCheckListItem(it, fTable, fWidget->fResolutionView);
	}
}

void ResolutionDlg::slotKeepBoth()
{
	if ( (fTable->fExistItems & eExistsPC) && (fTable->fExistItems & eExistsPalm) )
	{
		fTable->fResolution=SyncAction::eDuplicate;
	}
	else
	{
		fTable->fResolution=SyncAction::eDoNothing;
	}
	done(fTable->fResolution);
}

void ResolutionDlg::slotUseBackup()
{
	if (fTable->fExistItems & eExistsBackup)
	{
		fTable->fResolution=SyncAction::ePreviousSyncOverrides;
	}
	else
	{
		fTable->fResolution=SyncAction::eDoNothing;
	}
	done(fTable->fResolution);
}

void ResolutionDlg::slotUsePalm()
{
	if (fTable->fExistItems & eExistsPalm)
	{
		fTable->fResolution=SyncAction::eHHOverrides;
	}
	else
	{
		fTable->fResolution=SyncAction::eDelete;
	}
	done(fTable->fResolution);
}

void ResolutionDlg::slotUsePC()
{
	if (fTable->fExistItems & eExistsPC)
	{
		fTable->fResolution=SyncAction::ePCOverrides;
	}
	else
	{
		fTable->fResolution=SyncAction::eDelete;
	}
	done(fTable->fResolution);
}

void ResolutionDlg::slotApply()
{
	fTable->fResolution=SyncAction::eAskUser;
	done(fTable->fResolution);
}

void ResolutionDlg::_tickle()
{
	if (fHandle) fHandle->tickle();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ResolutionDlg::~ResolutionDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

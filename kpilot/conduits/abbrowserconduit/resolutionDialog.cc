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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <tqtimer.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlistview.h>
#include <tqregexp.h>

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
class ResolutionCheckListItem : TQCheckListItem {
public:
	ResolutionCheckListItem(ResolutionItem*it, ResolutionTable*tb,
		TQListView*parent);
	ResolutionCheckListItem(TQString header, TQString text,
		ResolutionCheckListItem*parent);
	~ResolutionCheckListItem() {};
	virtual void stateChange(bool newstate);
	virtual void setValue(TQString text);
	virtual void setCaption(TQString caption);

protected:
	void updateText();
	/* fResItem is only set for the controller */
	ResolutionItem*fResItem;
	bool isController;
	/* The description of the entry, e.g. Backup, PC, Palm for the radio buttons,
	 * of the field name for the controllers
	 */
	TQString fCaption;
	/* The currrent value of the entry (for controllers this changes with the
	 * selected button */
	TQString fText;
};


ResolutionCheckListItem::ResolutionCheckListItem(ResolutionItem*it,
		ResolutionTable*tb, TQListView*parent) :
	TQCheckListItem(parent, TQString::null, TQCheckListItem::Controller),
	fResItem(it),
	isController(true),
	fCaption(it?(it->fName):(TQString::null)),
	fText(it?(it->fResolved):(TQString::null))
{
	FUNCTIONSETUP;
	if (it && tb)
	{
		// If all three texts are identical, there is no need for
		// resolution so don't show the radio items below
		bool itemsEqual=true;
		TQString testtext(TQString::null);
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

ResolutionCheckListItem::ResolutionCheckListItem(TQString text, TQString header,
		ResolutionCheckListItem*parent) :
	TQCheckListItem(parent, TQString(), TQCheckListItem::RadioButton),
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

void ResolutionCheckListItem::setValue(TQString text)
{
	FUNCTIONSETUP;
	fText=text;
	if (isController && fResItem)
	{
		fResItem->fResolved=text;
	}
	updateText();
}

void ResolutionCheckListItem::setCaption(TQString caption)
{
	fCaption=caption;
	updateText();
}

void ResolutionCheckListItem::updateText()
{
	TQString newText(i18n("Entries in the resolution dialog. First the name of the field, then the entry from the Handheld or PC after the colon", "%1: %2").arg(fCaption).arg(fText));
	newText.replace(TQRegExp(CSL1("\n")),
		i18n("Denoting newlines in Address entries. No need to translate", " | "));
	setText(0, newText);
}



/*****************************************************************
 *
 *****************************************************************/

ResolutionDlg::ResolutionDlg( TQWidget* parent, KPilotLink*fH,
	const TQString &caption, const TQString &helpText, ResolutionTable*tab) :
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

	if (fHandle) tickleTimer=new TQTimer(this, "TickleTimer");

	if (tickleTimer)
	{
		connect( tickleTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(_tickle()));
		// tickle the palm every 10 seconds to prevent a timeout until the
		// sync is really finished.
		tickleTimer->start( 10000 );
	}

	connect(fWidget->fKeepBoth, TQT_SIGNAL(clicked()), TQT_SLOT(slotKeepBoth()));
	connect(fWidget->fBackupValues, TQT_SIGNAL(clicked()), TQT_SLOT(slotUseBackup()));
	connect(fWidget->fPalmValues, TQT_SIGNAL(clicked()), TQT_SLOT(slotUsePalm()));
	connect(fWidget->fPCValues, TQT_SIGNAL(clicked()), TQT_SLOT(slotUsePC()));
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
		DEBUGKPILOT<<"Building table, items="<<it->fExistItems<<", PC="<<
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

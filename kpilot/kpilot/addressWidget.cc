/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2004 by Adriaan de Groot <groot@kde.org>
**
** This file defines the addressWidget, that part of KPilot that
** displays address records from the Pilot.
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

#include <q3listbox.h>
#include <q3multilineedit.h>
#include <q3ptrlist.h>
#include <q3textstream.h>
#include <q3textview.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextcodec.h>

#include <kcombobox.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "kpilotConfig.h"
#include "listItems.h"
#include "pilotLocalDatabase.h"

#include "addressWidget.moc"


AddressWidget::AddressWidget(QWidget * parent,
	const QString & path) :
	PilotComponent(parent, "component_address", path),
	fAddrInfo(0L),
	fAddressAppInfo(0L)
{
	FUNCTIONSETUP;

	setupWidget();
	fAddressList.setAutoDelete(true);
}

AddressWidget::~AddressWidget()
{
	FUNCTIONSETUP;
}

int AddressWidget::getAllAddresses(PilotDatabase * addressDB)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotAddress *address;


	DEBUGKPILOT << "Reading AddressDB...";

	while ((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) &&
			(!(pilotRec->isSecret()) || KPilotSettings::showSecrets()))
		{
			address = new PilotAddress(pilotRec);
			if (address == 0L)
			{
				WARNINGKPILOT << "Couldn't allocate record"
					<< currentRecord++ ;
				break;
			}
			fAddressList.append(address);
		}
		delete pilotRec;

		currentRecord++;
	}

	DEBUGKPILOT << ": Total " << currentRecord << " records";

	return currentRecord;
}

void AddressWidget::showComponent()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << "Reading from directory [" << dbPath() << ']';

	PilotDatabase *addressDB =
		new PilotLocalDatabase(dbPath(), CSL1("AddressDB"));

	fAddressList.clear();

	if (addressDB->isOpen())
	{
		KPILOT_DELETE(fAddressAppInfo);
		fAddressAppInfo = new PilotAddressInfo(addressDB);
		populateCategories(fCatList, fAddressAppInfo->categoryInfo());
		getAllAddresses(addressDB);

	}
	else
	{
		populateCategories(fCatList, 0L);
		WARNINGKPILOT << "Could not open local AddressDB in [" << dbPath() << ']';
	}

	KPILOT_DELETE( addressDB );

	updateWidget();
}

void AddressWidget::hideComponent()
{
	FUNCTIONSETUP;
	fAddressList.clear();
	fListBox->clear();

	updateWidget();
}

/* virtual */ bool AddressWidget::preHotSync(QString &s)
{
	FUNCTIONSETUP;
	Q_UNUSED(s);

	return true;
}

void AddressWidget::postHotSync()
{
	FUNCTIONSETUP;

	if ( isVisible() )
	{
		fAddressList.clear();
		showComponent();
	}
}


void AddressWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label;
	QGridLayout *grid = new QGridLayout(this);
	grid->setMargin(SPACING);

	fCatList = new KComboBox(this);
	grid->addWidget(fCatList, 0, 1);
	connect(fCatList, SIGNAL(activated(int)),
		this, SLOT(slotSetCategory(int)));
	fCatList->setWhatsThis(
		i18n("<qt>Select the category of addresses to display here.</qt>"));

	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label, 0, 0);

	fListBox = new Q3ListBox(this);
	grid->addWidget(fListBox, 1, 0, 1, 2);
	connect(fListBox, SIGNAL(highlighted(int)),
		this, SLOT(slotShowAddress(int)));

	fListBox->setWhatsThis(
		i18n("<qt>This list displays all the addresses "
			"in the selected category. Click on "
			"one to display it to the right.</qt>"));

	label = new QLabel(i18n("Address info:"), this);
	grid->addWidget(label, 0, 2);

	// address info text view
	fAddrInfo = new Q3TextView(this);
	grid->addWidget(fAddrInfo, 1, 2, 3, 1);

	fExportButton = new QPushButton(i18nc("Export addresses to file","Export..."), this);
	grid->addWidget(fExportButton, 3,0,1,2);
	connect(fExportButton, SIGNAL(clicked()), this, SLOT(slotExport()));
	fExportButton->setWhatsThis(
		i18n("<qt>Export all addresses in the selected category to CSV format.</qt>") );

}

void AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	if( !fAddressAppInfo )
	{
		return;
	}

	int addressDisplayMode = KPilotSettings::addressDisplayMode();
	int listIndex = 0;
	int currentCatID = findSelectedCategory(fCatList,
		fAddressAppInfo->categoryInfo());

	fListBox->clear();
	fAddressList.first();

	while (fAddressList.current())
	{
		if ((currentCatID == -1) ||
			(fAddressList.current()->category() == currentCatID))
		{
			QString title = createTitle(fAddressList.current(),
				addressDisplayMode);

			if (!title.isEmpty())
			{
				title.remove(QRegExp(CSL1("\n.*")));
				PilotListItem *p = new PilotListItem(title,
					listIndex,
					fAddressList.current());

				fListBox->insertItem(p);
			}
		}
		listIndex++;
		fAddressList.next();
	}

	fListBox->sort();
	DEBUGKPILOT << listIndex << " records";

	slotUpdateButtons();
}



QString AddressWidget::createTitle(PilotAddress * address, int displayMode)
{
	QString title;

	switch (displayMode)
	{
	case 1:
		if (!address->getField(entryCompany).isEmpty())
		{
			title.append(address->getField(entryCompany));
		}
		if (!address->getField(entryLastname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}

			title.append(address->getField(entryLastname));
		}
		break;
	case 0:
	default:
		if (!address->getField(entryLastname).isEmpty())
		{
			title.append(address->getField(entryLastname));
		}

		if (!address->getField(entryFirstname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}
			title.append(address->getField(entryFirstname));
		}
		break;
	}

	if (title.isEmpty())	// One last try
	{
		if (!fAddressList.current()->getField(entryCompany).isEmpty())
		{
			title.append(fAddressList.current()->
				getField(entryCompany));
		}
		if (title.isEmpty())
		{
			title = i18nc("The title is inknown.", "[unknown]");
		}
	}

	return title;
}


/* slot */ void AddressWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	bool enabled = (fListBox->currentItem() != -1);

	fExportButton->setEnabled(enabled);

}

void AddressWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;

	updateWidget();
}

void AddressWidget::slotShowAddress(int which)
{
	FUNCTIONSETUP;
	if (!isVisible()) return;

	PilotListItem *p = (PilotListItem *) fListBox->item(which);
	PilotAddress *addr = (PilotAddress *) p->rec();

	DEBUGKPILOT << "Showing ["
		<< addr->getField(entryLastname)
		<< "] ["
		<< addr->getField(entryFirstname) << ']';

	QString text(CSL1("<qt>"));
	text += addr->getTextRepresentation(fAddressAppInfo,Qt::RichText);
	text += CSL1("</qt>\n");
	fAddrInfo->setText(text);

	slotUpdateButtons();
}


#define plu_quiet 1
#include "pilot-addresses.c"

void AddressWidget::slotExport()
{
	FUNCTIONSETUP;
	if( !fAddressAppInfo ) return;
	int currentCatID = findSelectedCategory(fCatList,
		fAddressAppInfo->categoryInfo());

	QString prompt = (currentCatID==-1) ?
		i18n("Export All Addresses") :
		i18n("Export Address Category %1",fAddressAppInfo->categoryName(currentCatID)) ;


	QString saveFile = KFileDialog::getSaveFileName(
		KUrl(),
		CSL1("*.csv|Comma Separated Values"),
		this,
		prompt
		);
	if (saveFile.isEmpty())
	{
		DEBUGKPILOT << "No save file selected.";
		return;
	}
	if (QFile::exists(saveFile) &&
		KMessageBox::warningContinueCancel(this,
			i18n("The file <i>%1</i> exists. Overwrite?",saveFile),
			i18n("Overwrite File?"),
			KGuiItem(i18n("Overwrite")))!=KMessageBox::Continue)
	{
		DEBUGKPILOT << "Overwrite file canceled.";
		return;
	}

	FILE *f = fopen(QFile::encodeName(saveFile),"w");
	if (!f)
	{
		KMessageBox::sorry(this,
			i18n("The file <i>%1</i> could not be opened for writing.",saveFile));
		return;
	}
	fAddressList.first();

	DEBUGKPILOT << "Adding records...";

	while (fAddressList.current())
	{
		const PilotAddress *a = fAddressList.current();
		if ((currentCatID == -1) ||
			(a->category() == currentCatID))
		{
			write_record_CSV(f, fAddressAppInfo->info(), a->address(),
				a->attributes(), a->category(), 0);
		}
		fAddressList.next();
	}

	fclose(f);
}


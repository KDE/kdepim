/* kpalmdoc_dlg.cpp
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This is the main dialog of the KDE PalmDOC converter.
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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kconfig.h>
#include <kaboutapplication.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kcharsets.h>

#include <pilotLocalDatabase.h>

#include "kpalmdoc_dlg.h"
#include "kpalmdoc_dlgbase.h"
#include "DOC-converter.h"
#include "kpalmdocSettings.h"


ConverterDlg::ConverterDlg( QWidget *parent, const QString& caption)
   : KDialogBase( parent, "converterdialog", false, caption, KDialogBase::Close|KDialogBase::Help|KDialogBase::User1,
	   KDialogBase::Close, true, i18n("&About"))
{
	QWidget *page = makeHBoxMainWidget();
	dlg=new ConverterDlgBase(page);
	QStringList l = KGlobal::charsets()->descriptiveEncodingNames();
	for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it)
	{
		dlg->fEncoding->insertItem(*it);
	}

	readSettings();

	connect(dlg->fDirectories, SIGNAL(toggled(bool)),
		this, SLOT(slotDirectories(bool)));
	connect(dlg->fTextToPDB, SIGNAL(clicked()), this, SLOT(slotToPDB()));
	connect(dlg->fPDBToText, SIGNAL(clicked()), this, SLOT(slotToText()));

	resize(minimumSize());
}

ConverterDlg::~ConverterDlg()
{
    // no need to delete child widgets, Qt does it all for us
}
void ConverterDlg::writeSettings()
{
	// General page
	KPalmDocSettings::setTXTFolder( dlg->fTXTDir->url() );
	KPalmDocSettings::setPDBFolder( dlg->fPDBDir->url() );
	KPalmDocSettings::setSyncFolders( dlg->fDirectories->isChecked() );
	KPalmDocSettings::setAskOverwrite( dlg->fAskOverwrite->isChecked() );
	KPalmDocSettings::setVerboseMessages( dlg->fVerbose->isChecked() );
	KPalmDocSettings::setEncoding( dlg->fEncoding->currentText() );

	// PC->Handheld page
	KPalmDocSettings::setCompress( dlg->fCompress->isChecked() );
	KPalmDocSettings::setConvertBookmarks( dlg->fConvertBookmarks->isChecked() );
	KPalmDocSettings::setBookmarksInline( dlg->fBookmarksInline->isChecked() );
	KPalmDocSettings::setBookmarksEndtags( dlg->fBookmarksEndtags->isChecked() );
	KPalmDocSettings::setBookmarksBmk( dlg->fBookmarksBmk->isChecked() );

	// Handheld->PC page
	KPalmDocSettings::setBookmarksToPC( dlg->fPCBookmarks->id(dlg->fPCBookmarks->selected()) );

	KPalmDocSettings::self()->writeConfig();
}

void ConverterDlg::readSettings()
{
	FUNCTIONSETUP;

	KPalmDocSettings::self()->readConfig();

	// General Page:
	dlg->fTXTDir->setURL(KPalmDocSettings::tXTFolder());
	dlg->fPDBDir->setURL(KPalmDocSettings::pDBFolder());
	bool dir=KPalmDocSettings::syncFolders();
	dlg->fDirectories->setChecked(dir);
	slotDirectories(dir);
	dlg->fAskOverwrite->setChecked( KPalmDocSettings::askOverwrite() );
	dlg->fVerbose->setChecked( KPalmDocSettings::verboseMessages() );
	QString encoding = KPalmDocSettings::encoding();
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Encoding=" << encoding << endl;
#endif
	dlg->fEncoding->setCurrentText( KPalmDocSettings::encoding() );

	// PC->Handheld page
	dlg->fCompress->setChecked(KPalmDocSettings::compress() );
	dlg->fConvertBookmarks->setChecked(KPalmDocSettings::convertBookmarks());
	dlg->fBookmarksInline->setChecked(KPalmDocSettings::bookmarksInline());
	dlg->fBookmarksEndtags->setChecked(KPalmDocSettings::bookmarksEndtags());
	dlg->fBookmarksBmk->setChecked(KPalmDocSettings::bookmarksBmk());

	// Handheld->PC page
	dlg->fPCBookmarks->setButton(KPalmDocSettings::bookmarksToPC() );
}

void ConverterDlg::slotClose()
{
  writeSettings();
  kapp->quit();
  delete this;
}

void ConverterDlg::slotToText()
{
	FUNCTIONSETUP;
	// First, get the settings from the controls and initialize
	// the converter object
	int bmks=dlg->fPCBookmarks->id(dlg->fPCBookmarks->selected());
	DOCConverter conv;
	switch(bmks) {
		case 0: conv.setBookmarkTypes(DOCConverter::eBmkNone); break;
		case 1: conv.setBookmarkTypes(DOCConverter::eBmkInline); break;
		case 2: conv.setBookmarkTypes(DOCConverter::eBmkEndtags); break;
		case 3: conv.setBookmarkTypes(DOCConverter::eBmkDefaultBmkFile); break;
		default:
			break;
	}

	askOverwrite=dlg->fAskOverwrite->isChecked();
	verbose=dlg->fVerbose->isChecked();


	bool dir=dlg->fDirectories->isChecked();
	QString txturl=dlg->fTXTDir->url();
	QString pdburl=dlg->fPDBDir->url();

	QFileInfo txtinfo(txturl);
	QFileInfo pdbinfo(pdburl);

	if (dir)
	{
		if (pdbinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this,
				i18n("<qt>You selected to sync folders, "
				"but gave a filename instead (<em>%1</em>)."
				"<br>Use folder <em>%2</em> instead?</qt>").arg(pdburl)
				.arg(pdbinfo.dirPath(true)));
			if (res==KMessageBox::Yes)
			{
				pdburl=pdbinfo.dirPath(true);
				pdbinfo.setFile(pdburl);
			}
			else return;
		}

		if (!pdbinfo.isDir())
		{
			// no directory, so error message and return
			KMessageBox::sorry(this,
				i18n("<qt>The folder <em>%1</em> for "
				"the handheld database files is not a valid "
				"folder.</qt>").arg(pdburl));
			return;
		}

		if (!pdbinfo.exists())
		{
			KMessageBox::sorry(this,
				i18n("<qt>The folder <em>%1</em> for "
				"the handheld database files is not a "
				"valid directory.</qt>").arg(pdburl));
			return;
		}


		// Now check the to directory:
		if (txtinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this,
				i18n("<qt>You selected to sync folders, "
				"but gave a filename instead (<em>%1</em>)."
				"<br>Use folder <em>%2</em> instead?</qt>").arg(txturl)
				.arg(txtinfo.dirPath(true)));
			if (res==KMessageBox::Yes) {
				txturl=txtinfo.dirPath(true);
				txtinfo.setFile(txturl);
			}
			else return;
		}

		// Now that we have a directory path, try to create it:
		if (!txtinfo.isDir()) {
			txtinfo.dir().mkdir(txturl, true);
		}
		if (!txtinfo.isDir()) {
			KMessageBox::sorry(this,
				i18n("<qt>The folder <em>%1</em> for "
				"the text files could not be created.</qt>").arg(txturl));
			return;
		}


		// Now that we have both directories, create the converter object
		DEBUGCONDUIT<<"Pdbinfo.dir="<<pdbinfo.dir().absPath()<<endl;
		DEBUGCONDUIT<<"txtinfo.dir="<<txtinfo.dir().absPath()<<endl;
		QStringList pdbfiles(pdbinfo.dir().entryList("*.pdb"));
		QStringList converted_Files;

		DEBUGCONDUIT<<"Length of filename list: "<<pdbfiles.size()<<endl;
		for ( QStringList::Iterator it = pdbfiles.begin(); it != pdbfiles.end(); ++it )
		{
			QString txtfile=QFileInfo(*it).baseName(true)+".txt";
			DEBUGCONDUIT<<"pdbfile="<<*it<<", pdbdir="<<pdburl<<", txtfile="<<txtfile<<", txtdir="<<txturl<<endl;
			if (convertPDBtoTXT(pdburl, *it, txturl, txtfile, &conv))
			{
				converted_Files.append(*it);
			}
		}
		if (converted_Files.size()>0) {
			KMessageBox::informationList(this, i18n("The following texts were "
					"successfully converted:"), converted_Files, i18n("Conversion Successful"));
		}
		else
		{
			KMessageBox::sorry(this, i18n("No text files were converted correctly"));
		}


	} else { // no dir


		// Check the from file
		if (!pdbinfo.isFile() || !pdbinfo.exists())
		{
			KMessageBox::sorry(this, i18n("<qt>The file <em>%1</em> does not "
				"exist.</qt>").arg(pdburl));
			return;
		}

		// Now check the to file
/*		// I can't check if a given filename is a valid filename
		if (!txtinfo.isFile())
		{
			KMessageBox::sorry(this, i18n("<qt>The filename <em>%1</em> for the "
				"text is not a valid filename.</qt>").arg(txturl));
			return;
		}*/
		if (convertPDBtoTXT(pdbinfo.dirPath(true), pdbinfo.fileName(),
				txtinfo.dirPath(true), txtinfo.fileName(), &conv) )
		{
			KMessageBox::information(this, i18n("Conversion of file %1 successful.").arg(pdburl));
		}

	}

}

void ConverterDlg::slotToPDB()
{
	FUNCTIONSETUP;
	// First, get the settings from the controls and initialize
	// the converter object
	bool compress=dlg->fCompress->isChecked();
	int bmks=0;
	if (dlg->fConvertBookmarks->isChecked())
	{
		if (dlg->fBookmarksInline->isChecked()) bmks|=DOCConverter::eBmkInline;
		if (dlg->fBookmarksEndtags->isChecked()) bmks|=DOCConverter::eBmkEndtags;
		if(dlg->fBookmarksBmk->isChecked()) bmks|=DOCConverter::eBmkDefaultBmkFile;
	}
	DOCConverter conv;
	conv.setBookmarkTypes(bmks);
	conv.setCompress(compress);
	conv.setSort(DOCConverter::eSortName);


	askOverwrite=dlg->fAskOverwrite->isChecked();
	verbose=dlg->fVerbose->isChecked();


	bool dir=dlg->fDirectories->isChecked();
	QString txturl=dlg->fTXTDir->url();
	QString pdburl=dlg->fPDBDir->url();

	QFileInfo txtinfo(txturl);
	QFileInfo pdbinfo(pdburl);

	if (dir)
	{
		if (txtinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this,
				i18n("<qt>You selected to sync folders, "
				"but gave a filename instead (<em>%1</em>)."
				"<br>Use folder <em>%2</em> instead?</qt>").arg(txturl)
				.arg(txtinfo.dirPath(true)));
			if (res==KMessageBox::Yes)
			{
				txturl=txtinfo.dirPath(true);
				txtinfo.setFile(txturl);
			}
			else return;
		}

		if (!txtinfo.isDir() || !txtinfo.exists())
		{
			KMessageBox::sorry(this,
				i18n("<qt>The folder <em>%1</em> for "
				"the text files is not a valid folder.</qt>").arg(txturl));
			return;
		}


		// Now check the to directory:
		if (pdbinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this,
				i18n("<qt>You selected to sync folders, "
				"but gave a filename instead (<em>%1</em>)."
				"<br>Use folder <em>%2</em> instead?</qt>")
				.arg(pdburl)
				.arg(pdbinfo.dirPath(true)));
			if (res==KMessageBox::Yes) {
				pdburl=pdbinfo.dirPath(true);
				pdbinfo.setFile(pdburl);
			}
			else return;
		}

		// Now that we have a directory path, try to create it:
		if (!pdbinfo.isDir()) {
			pdbinfo.dir().mkdir(pdburl, true);
		}
		if (!pdbinfo.isDir()) {
			KMessageBox::sorry(this, i18n("<qt>The folder <em>%1</em> for "
				"the PalmDOC files could not be created.</qt>").arg(pdburl));
			return;
		}


		// Now that we have both directories, create the converter object
		DEBUGCONDUIT<<"Pdbinfo.dir="<<pdbinfo.dir().absPath()<<endl;
		DEBUGCONDUIT<<"txtinfo.dir="<<txtinfo.dir().absPath()<<endl;
		QStringList txtfiles(txtinfo.dir().entryList("*.txt"));
		QStringList converted_Files;

		DEBUGCONDUIT<<"Length of filename list: "<<txtfiles.size()<<endl;
		for ( QStringList::Iterator it = txtfiles.begin(); it != txtfiles.end(); ++it )
		{
			QString pdbfile=QFileInfo(*it).baseName(true)+".pdb";
			DEBUGCONDUIT<<"pdbfile="<<pdbfile<<", pdbdir="<<pdburl<<", txtfile="<<*it<<", txtdir="<<txturl<<endl;
			if (convertTXTtoPDB(txturl, *it, pdburl, pdbfile, &conv))
			{
				converted_Files.append(*it);
			}
		}
		if (converted_Files.size()>0) {
			KMessageBox::informationList(this, i18n("The following texts were "
					"successfully converted:"), converted_Files, i18n("Conversion Successful"));
		}
		else
		{
			KMessageBox::sorry(this, i18n("No text files were converted correctly"));
		}


	} else { // no dir


		// Check the from file
		if (!txtinfo.isFile() || !txtinfo.exists())
		{
			KMessageBox::sorry(this, i18n("<qt>The file <em>%1</em> does not "
				"exist.</qt>").arg(txturl));
			return;
		}

		if (convertTXTtoPDB(txtinfo.dirPath(true), txtinfo.fileName(),
				pdbinfo.dirPath(true), pdbinfo.fileName(), &conv) )
		{
			KMessageBox::information(this, i18n("Conversion of file %1 successful.").arg(txturl));
		}

	}

}


void ConverterDlg::slotUser1()
{
	KAboutApplication ab(KGlobal::instance()->aboutData(), this);
	ab.show();
	ab.exec();
	return;
}

void ConverterDlg::slotDirectories(bool dir)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<"Slot Directories: "<<dir<<endl;
	if (dir)
	{
		dlg->fTextLabel->setText(i18n("&Text folder:"));
		dlg->fPdbLabel->setText(i18n("&PalmDOC folder:"));
		dlg->fTXTDir->setMode(KFile::LocalOnly | KFile::Directory);
		dlg->fPDBDir->setMode(KFile::LocalOnly | KFile::Directory);
	} else {
		dlg->fTextLabel->setText(i18n("&Text file:"));
		dlg->fPdbLabel->setText(i18n("&DOC file:"));
		dlg->fTXTDir->setMode(KFile::LocalOnly | KFile::File);
		dlg->fPDBDir->setMode(KFile::LocalOnly | KFile::File);
	}
}

bool ConverterDlg::convertTXTtoPDB(QString txtdir, QString txtfile,
		QString pdbdir, QString pdbfile, DOCConverter*conv)
{
	FUNCTIONSETUP;
	bool res=false;
	QFileInfo dbfileinfo(pdbdir, pdbfile);
	DEBUGCONDUIT<<"Working  on file "<<pdbfile<<endl;
	if (!dbfileinfo.exists() || !askOverwrite ||
			(KMessageBox::Yes==KMessageBox::questionYesNo(this,
			i18n("<qt>The database file <em>%1</em> already exists. Overwrite it?</qt>")
			.arg(dbfileinfo.filePath()) ) ))
	{
		PilotLocalDatabase*pdbdb=new PilotLocalDatabase(pdbdir, QFileInfo(pdbfile).baseName(), false);
		if (pdbdb)
		{
			if (!pdbdb->isDBOpen())
			{
#ifdef DEBUG
				DEBUGCONDUIT<<pdbfile<<" does not yet exist. Creating it"<<endl;
#endif
				if (!pdbdb->createDatabase(get_long("REAd"), get_long("TEXt")) ) {
				}
			}

			if (pdbdb->isDBOpen())
			{
				conv->setPDB(pdbdb);
				conv->setTXTpath(txtdir, txtfile);
				DEBUGCONDUIT<<"Converting "<<txtfile<<" (dir "<<txtdir<<") to "<<dbfileinfo.filePath()<<endl;
				if (conv->convertTXTtoPDB()) res=true;
			}
			delete pdbdb;
		}
		if ( !res && verbose )
		{
			KMessageBox::sorry(this, i18n("<qt>Error while converting the text %1.</qt>").arg(txtfile));
		}
	}
	else
	{
		DEBUGCONDUIT<<"Ignoring the file "<<txtfile<<endl;
	}
	return res;
}

bool ConverterDlg::convertPDBtoTXT(QString pdbdir, QString pdbfile,
		QString txtdir, QString txtfile, DOCConverter*conv)
{
	FUNCTIONSETUP;
	bool res=false;
	QFileInfo txtfileinfo(txtdir, txtfile);
	DEBUGCONDUIT<<"Working  on file "<<txtfile<<endl;
	if (!txtfileinfo.exists() || !askOverwrite ||
			(KMessageBox::Yes==KMessageBox::questionYesNo(this,
			i18n("<qt>The text file <em>%1</em> already exists. Overwrite it?</qt>")
			.arg(txtfileinfo.filePath()) ) ))
	{
		PilotLocalDatabase*pdbdb=new PilotLocalDatabase(pdbdir, QFileInfo(pdbfile).baseName(), false);
		if (pdbdb)
		{
			if (pdbdb->isDBOpen())
			{
				conv->setPDB(pdbdb);
				conv->setTXTpath(txtdir, txtfile);
				DEBUGCONDUIT<<"Converting "<<txtfile<<" (dir "<<txtdir<<") from "<<pdbfile<<" (dir "<<pdbdir<<")"<<endl;
				if (conv->convertPDBtoTXT()) res=true;
			}
			delete pdbdb;
		}
		if ( !res && verbose )
		{
			KMessageBox::sorry(this, i18n("<qt>Error while converting the text %1.</qt>").arg(pdbfile));
		}
	}
	else
	{
		DEBUGCONDUIT<<"Ignoring the file "<<pdbfile<<endl;
	}
	return res;

}

#include "kpalmdoc_dlg.moc"

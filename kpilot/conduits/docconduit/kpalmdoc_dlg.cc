#include "kpalmdoc_dlg.h"
#include "kpalmdoc_dlgbase.h"

#include <qtabwidget.h>
#include <kapplication.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <kurlrequester.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <qbuttongroup.h>
#include <klocale.h>
#include <qlabel.h>
#include <kaboutapplication.h>
#include <kmessagebox.h>

#include <pilotLocalDatabase.h>

#include "options.h"
#include "DOC-converter.h"


ConverterDlg::ConverterDlg( QWidget *parent, const QString& caption)
   : KDialogBase( parent, "converterdialog", false, caption, KDialogBase::Close|KDialogBase::Help|KDialogBase::User1,
	   KDialogBase::Close, true, i18n("&About"))
{
	QWidget *page = makeHBoxMainWidget();
	dlg=new ConverterDlgBase(page);
	readSettings();
//	setMainWidget(dlg->tabWidget);

	// TODO: Connect the signals and slots from the various widgets!
	// e.g.
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
	KConfig* fConfig = kapp->config();
	if (!fConfig) return;
//	config->setGroup("GeneralData");

	// General page
	fConfig->writeEntry("TXT folder", dlg->fTXTDir->url());
	fConfig->writeEntry("PDB folder", dlg->fPDBDir->url());
	fConfig->writeEntry("Sync folders", dlg->fDirectories->isChecked());
	fConfig->writeEntry("Ask before overwriting files", dlg->fAskOverwrite->isChecked());
	fConfig->writeEntry("Verbose messages", dlg->fVerbose->isChecked());

	// PC->Handheld page
	fConfig->writeEntry("Compress", dlg->fCompress->isChecked());
	fConfig->writeEntry("Convert bookmarks", dlg->fConvertBookmarks->isChecked());
	fConfig->writeEntry("Bookmarks inline", dlg->fBookmarksInline->isChecked());
	fConfig->writeEntry("Bookmarks endtags", dlg->fBookmarksEndtags->isChecked());
	fConfig->writeEntry("Bookmarks bmk", dlg->fBookmarksBmk->isChecked());

	// Handheld->PC page
	fConfig->writeEntry("Bookmarks to PC",
		dlg->fPCBookmarks->id(dlg->fPCBookmarks->selected()));

	fConfig->sync();
}
void ConverterDlg::readSettings()
{
	KConfig* fConfig = kapp->config();
	if (!fConfig) return;

	// General Page:
	dlg->fTXTDir->setURL(fConfig->
		readEntry("TXT directory"));
	dlg->fPDBDir->setURL(fConfig->
		readEntry("PDB directory"));
	bool dir=fConfig->
		readBoolEntry("Sync directories", false);
	dlg->fDirectories->setChecked(dir);
	slotDirectories(dir);
	askOverwrite=fConfig->readBoolEntry("Ask before overwriting files", true);
	dlg->fAskOverwrite->setChecked(askOverwrite);
	verbose=fConfig->readBoolEntry("Verbose messages", true);
	dlg->fVerbose->setChecked(verbose);

	// PC->Handheld page
	dlg->fCompress->setChecked(fConfig->
		readBoolEntry("Compress", true));
	dlg->fConvertBookmarks->setChecked(fConfig->
		readBoolEntry("Convert bookmarks", true));
	dlg->fBookmarksInline->setChecked(fConfig->
		readBoolEntry("Bookmarks inline", true));
	dlg->fBookmarksEndtags->setChecked(fConfig->
		readBoolEntry("Bookmarks endtags", true));
	dlg->fBookmarksBmk->setChecked(fConfig->
		readBoolEntry("Bookmarks bmk", true));

	// Handheld->PC page
	dlg->fPCBookmarks->setButton(fConfig->
		readNumEntry("Bookmarks to PC", 0));
}

void ConverterDlg::slotClose()
{
  writeSettings();
  kapp->quit();
  delete this;
}

void ConverterDlg::slotToText()
{
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
					"successfully converted:"), converted_Files, i18n("Conversion successful"));
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
			int res=KMessageBox::questionYesNo(this, i18n("<qt>You selected "
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
					"successfully converted:"), converted_Files, i18n("Conversion successful"));
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
	bool res=false;
	QFileInfo dbfileinfo(pdbdir, pdbfile);
DEBUGCONDUIT<<"Working  on file "<<pdbfile<<endl;
	if (!dbfileinfo.exists() || !askOverwrite ||
			(KMessageBox::Yes==KMessageBox::questionYesNo(this,
			i18n("<qt>The database file already <em>%1</em> exists.  Overwrite it?</qt>")
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
	bool res=false;
	QFileInfo txtfileinfo(txtdir, txtfile);
DEBUGCONDUIT<<"Working  on file "<<txtfile<<endl;
	if (!txtfileinfo.exists() || !askOverwrite ||
			(KMessageBox::Yes==KMessageBox::questionYesNo(this,
			i18n("<qt>The text file already <em>%1</em> exists.  Overwrite it?</qt>")
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

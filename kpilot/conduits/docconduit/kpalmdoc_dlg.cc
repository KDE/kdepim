#include "kpalmdoc_dlg.h"
#include "kpalmdoc_dlgbase.h"

#include <qtabwidget.h>
#include <kapplication.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

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
	connect(dlg->fTextToDoc, SIGNAL(clicked()), this, SLOT(slotToDoc()));
	connect(dlg->fDocToText, SIGNAL(clicked()), this, SLOT(slotToText()));

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
	fConfig->writeEntry("DOC directory", dlg->fDOCDir->url());
	fConfig->writeEntry("PDB directory", dlg->fPDBDir->url());
	fConfig->writeEntry("Sync directories", dlg->fDirectories->isChecked());
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
	dlg->fDOCDir->setURL(fConfig->
		readEntry("DOC directory", QString::null));
	dlg->fPDBDir->setURL(fConfig->
		readEntry("PDB directory", QString::null));
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
	QString docurl=dlg->fDOCDir->url();
	QString pdburl=dlg->fPDBDir->url();

	QFileInfo docinfo(docurl);
	QFileInfo pdbinfo(pdburl);

	if (dir)
	{
		if (pdbinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this, i18n("<qt>You selected "
				"directory sync, but gave a filename <em>%1</em>. <br>Use "
				"directory <em>%2</em instead?</qt>").arg(pdburl)
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
			KMessageBox::sorry(this, i18n("<qt>The directory <em>%1</em> for "
				"the handheld database files is not a valid directory.</qt>").arg(pdburl));
			return;
		}

		if (!pdbinfo.exists())
		{
			KMessageBox::sorry(this, i18n("<qt>The directory <em>%1</em> for "
				"the handheld database files is not a valid directory.</qt>").arg(pdburl));
			return;
		}


		// Now check the to directory:
		if (docinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this, i18n("<qt>You selected "
				"directory sync, but gave a filename <em>%1</em>. <br>Use "
				"directory <em>%2</em instead?</qt>").arg(docurl)
				.arg(docinfo.dirPath(true)));
			if (res==KMessageBox::Yes) {
				docurl=docinfo.dirPath(true);
				docinfo.setFile(docurl);
			}
			else return;
		}

		// Now that we have a directory path, try to create it:
		if (!docinfo.isDir()) {
			docinfo.dir().mkdir(docurl, true);
		}
		if (!docinfo.isDir()) {
			KMessageBox::sorry(this, i18n("<qt>The directory <em>%1</em> for "
				"the text files could not be created.</qt>").arg(docurl));
			return;
		}


		// Now that we have both directories, create the converter object
cout<<"Pdbinfo.dir="<<pdbinfo.dir().absPath()<<endl;
cout<<"txtinfo.dir="<<docinfo.dir().absPath()<<endl;
		QStringList pdbfiles(pdbinfo.dir().entryList("*.pdb"));
		QStringList converted_Files;

cout<<"Length of filename list: "<<pdbfiles.size()<<endl;
		for ( QStringList::Iterator it = pdbfiles.begin(); it != pdbfiles.end(); ++it )
		{
			QString docfile=QFileInfo(*it).baseName(true)+".txt";
cout<<"pdbfile="<<*it<<", pdbdir="<<pdburl<<", docfile="<<docfile<<", docdir="<<docurl<<endl;
			if (convertPDBToText(pdburl, *it, docurl, docfile, &conv))
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
		if (!docinfo.isFile())
		{
			KMessageBox::sorry(this, i18n("<qt>The filename <em>%1</em> for the "
				"text is not a valid filename.</qt>").arg(docurl));
			return;
		}*/
		if (convertPDBToText(pdbinfo.dirPath(true), pdbinfo.fileName(),
				docinfo.dirPath(true), docinfo.fileName(), &conv) )
		{
			KMessageBox::information(this, i18n("Conversion of file %1 successful.").arg(pdburl));
		}

	}

}

void ConverterDlg::slotToDoc()
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
	QString docurl=dlg->fDOCDir->url();
	QString pdburl=dlg->fPDBDir->url();

	QFileInfo docinfo(docurl);
	QFileInfo pdbinfo(pdburl);

	if (dir)
	{
		if (docinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this, i18n("<qt>You selected "
				"directory sync, but gave a filename <em>%1</em>. <br>Use "
				"directory <em>%2</em instead?</qt>").arg(docurl)
				.arg(docinfo.dirPath(true)));
			if (res==KMessageBox::Yes)
			{
				docurl=docinfo.dirPath(true);
				docinfo.setFile(docurl);
			}
			else return;
		}

		if (!docinfo.isDir() || !docinfo.exists())
		{
			KMessageBox::sorry(this, i18n("<qt>The directory <em>%1</em> for "
				"the text files is not a valid directory.</qt>").arg(docurl));
			return;
		}


		// Now check the to directory:
		if (pdbinfo.isFile())
		{
			int res=KMessageBox::questionYesNo(this, i18n("<qt>You selected "
				"directory sync, but gave a filename <em>%1</em>. <br>Use "
				"directory <em>%2</em instead?</qt>").arg(pdburl)
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
			KMessageBox::sorry(this, i18n("<qt>The directory <em>%1</em> for "
				"the PalmDOC files could not be created.</qt>").arg(pdburl));
			return;
		}


		// Now that we have both directories, create the converter object
cout<<"Pdbinfo.dir="<<pdbinfo.dir().absPath()<<endl;
cout<<"txtinfo.dir="<<docinfo.dir().absPath()<<endl;
		QStringList txtfiles(docinfo.dir().entryList("*.txt"));
		QStringList converted_Files;

cout<<"Length of filename list: "<<txtfiles.size()<<endl;
		for ( QStringList::Iterator it = txtfiles.begin(); it != txtfiles.end(); ++it )
		{
			QString pdbfile=QFileInfo(*it).baseName(true)+".pdb";
cout<<"pdbfile="<<pdbfile<<", pdbdir="<<pdburl<<", docfile="<<*it<<", docdir="<<docurl<<endl;
			if (convertTextToPDB(docurl, *it, pdburl, pdbfile, &conv))
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
		if (!docinfo.isFile() || !docinfo.exists())
		{
			KMessageBox::sorry(this, i18n("<qt>The file <em>%1</em> does not "
				"exist.</qt>").arg(docurl));
			return;
		}

		if (convertTextToPDB(pdbinfo.dirPath(true), pdbinfo.fileName(),
				docinfo.dirPath(true), docinfo.fileName(), &conv) )
		{
			KMessageBox::information(this, i18n("Conversion of file %1 successful.").arg(docurl));
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
	cout<<"Slot Directories: "<<dir<<endl;
	if (dir)
	{
		dlg->fTextLabel->setText(i18n("&Text directory:"));
		dlg->fDocLabel->setText(i18n("&DOC directory:"));
		dlg->fDOCDir->setMode(KFile::LocalOnly | KFile::Directory);
		dlg->fPDBDir->setMode(KFile::LocalOnly | KFile::Directory);
	} else {
		dlg->fTextLabel->setText(i18n("&Text file:"));
		dlg->fDocLabel->setText(i18n("&DOC file:"));
		dlg->fDOCDir->setMode(KFile::LocalOnly | KFile::File);
		dlg->fPDBDir->setMode(KFile::LocalOnly | KFile::File);
	}
}

bool ConverterDlg::convertTextToPDB(QString docdir, QString docfile,
		QString pdbdir, QString pdbfile, DOCConverter*conv)
{
	bool res=false;
	QFileInfo dbfileinfo(pdbdir, pdbfile);
cout<<"Working  on file "<<pdbfile<<endl;
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
				conv->setDOCpath(docdir, docfile);
				cout<<"Converting "<<docfile<<" (dir "<<docdir<<") to "<<dbfileinfo.filePath()<<endl;
				if (conv->convertDOCtoPDB()) res=true;
			}
			delete pdbdb;
		}
		if ( !res && verbose )
		{
			KMessageBox::sorry(this, i18n("<qt>Error while converting the text %1.</qt>").arg(docfile));
		}
	}
	else
	{
		cout<<"Ignoring the file "<<docfile<<endl;
	}
	return res;
}

bool ConverterDlg::convertPDBToText(QString pdbdir, QString pdbfile,
		QString docdir, QString docfile, DOCConverter*conv)
{
	bool res=false;
	QFileInfo docfileinfo(docdir, docfile);
cout<<"Working  on file "<<docfile<<endl;
	if (!docfileinfo.exists() || !askOverwrite ||
			(KMessageBox::Yes==KMessageBox::questionYesNo(this,
			i18n("<qt>The text file already <em>%1</em> exists.  Overwrite it?</qt>")
			.arg(docfileinfo.filePath()) ) ))
	{
		PilotLocalDatabase*pdbdb=new PilotLocalDatabase(pdbdir, QFileInfo(pdbfile).baseName(), false);
		if (pdbdb)
		{
			if (pdbdb->isDBOpen())
			{
				conv->setPDB(pdbdb);
				conv->setDOCpath(docdir, docfile);
				cout<<"Converting "<<docfile<<" (dir "<<docdir<<") from "<<pdbfile<<" (dir "<<pdbdir<<")"<<endl;
				if (conv->convertPDBtoDOC()) res=true;
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
		cout<<"Ignoring the file "<<pdbfile<<endl;
	}
	return res;

}

#include "kpalmdoc_dlg.moc"

/*
    ktnefmain.cpp

    Copyright (C) 2002 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "ktnefmain.h"
#include <ktnef/ktnefparser.h>
#include "ktnefview.h"
#include <ktnef/ktnefattach.h>
#include <ktnef/ktnefproperty.h>
#include <ktnef/ktnefmessage.h>
#include "attachpropertydialog.h"
#include "messagepropertydialog.h"

#include <qpopupmenu.h>
#include <klistview.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <qpixmap.h>
#include <kstdaccel.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <qdir.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qinputdialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <ktempfile.h>
#include <kdebug.h>
#ifdef KDE_NO_COMPAT
#undef KDE_NO_COMPAT
#endif

#include <krun.h>
#include <kopenwith.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>


#define	NOT_IMPLEMENTED QMessageBox::information(this, "ktnef", "Not implemented yet", QMessageBox::Ok|QMessageBox::Default, 0)

KFileOpenWithHandler*	KTNEFMain::handler_ = 0;

KTNEFMain::KTNEFMain(QWidget *parent, const char *name)
	: KMainWindow(parent, name)
{
	setupActions();
	setupStatusbar();

	setupTNEF();

	KGlobal::config()->setGroup("Settings");
	defaultdir_ = KGlobal::config()->readEntry("defaultdir","/tmp/");
	lastdir_ = defaultdir_;

	// create personale temo extract dir
	KStandardDirs::makeDir(KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp");

	// create an instance of KFIleOpenWithHandler
	if (!handler_) handler_ = new KFileOpenWithHandler();

	resize(430,350);
	setAutoSaveSettings( "MainWindow" );
}

KTNEFMain::~KTNEFMain()
{
	delete parser_;
	if (handler_ && memberList->count() <= 1)
	{
		delete handler_;
		handler_ = 0;
	}
	cleanup();
}

void KTNEFMain::setupActions()
{
	// File menu
	KStdAction::open(this, SLOT(openFile()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	// Action menu
	new KAction(i18n("View"), QString("viewmag"), 0, this, SLOT(viewFile()), actionCollection(), "view_file");
	new KAction(i18n("View With..."), QString("package_applications"), 0, this, SLOT(viewFileAs()), actionCollection(), "view_file_as");
	new KAction(i18n("Extract"), 0, this, SLOT(extractFile()), actionCollection(), "extract_file");
	new KAction(i18n("Extract To..."), QString("ktnef_extract_to"), 0, this, SLOT(extractFileTo()), actionCollection(), "extract_file_to");
	new KAction(i18n("Extract All To..."), QString("ktnef_extract_all_to"), 0, this, SLOT(extractAllFiles()), actionCollection(), "extract_all_files");
	new KAction( i18n( "Message Properties..." ), "help", 0, this, SLOT( slotShowMessageProperties() ), actionCollection(), "msg_properties" );
	new KAction(i18n("Properties..."), QString("contents"), 0, this, SLOT(propertiesFile()), actionCollection(), "properties_file");
	new KAction( i18n( "Show Message Text" ), "mail_generic", 0, this, SLOT( slotShowMessageText() ), actionCollection(), "msg_text" );
	new KAction( i18n( "Save Message Text..." ), "filesave", 0, this, SLOT( slotSaveMessageText() ), actionCollection(), "msg_save" );
	actionCollection()->action("view_file")->setEnabled(false);
	actionCollection()->action("view_file_as")->setEnabled(false);
	actionCollection()->action("extract_file")->setEnabled(false);
	actionCollection()->action("extract_file_to")->setEnabled(false);
	actionCollection()->action("extract_all_files")->setEnabled(false);
	actionCollection()->action("properties_file")->setEnabled(false);

	// Option menu
	new KAction(i18n("Default Folder..."), QString("folder_open"), 0, this, SLOT(optionDefaultDir()), actionCollection(), "options_default_dir");

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	KStdAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());

	createGUI();
}

void KTNEFMain::setupStatusbar()
{
	statusBar()->insertItem(i18n("100 attachments found"), 0);
	statusBar()->changeItem(i18n("No file loaded"), 0);
}

void KTNEFMain::setupTNEF()
{
	view_ = new KTNEFView(this);
	view_->setAllColumnsShowFocus( true );
	parser_ = new KTNEFParser;

	setCentralWidget(view_);
	connect(view_, SIGNAL(selectionChanged()), SLOT(viewSelectionChanged()));
	connect(view_, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,int)), SLOT(viewRightButtonPressed(QListViewItem*,const QPoint&,int)));
	connect(view_, SIGNAL(doubleClicked(QListViewItem*)), SLOT(viewDoubleClicked(QListViewItem*)));
}

void KTNEFMain::loadFile(const QString& filename)
{
	filename_ = filename;
	setCaption(filename_);
	if (!parser_->openFile(filename))
	{
		QMessageBox::critical(this, i18n("Error"), i18n("Unable to open file"), QMessageBox::Ok|QMessageBox::Default, 0);
		view_->setAttachments(0);
		enableExtractAll(false);
	}
	else
	{
		QPtrList<KTNEFAttach>	list = parser_->message()->attachmentList();
		QString			msg;
		msg = i18n( "%1 attachments found" ).arg( list.count() );
		statusBar()->changeItem(msg, 0);
		view_->setAttachments(&list);
		enableExtractAll((list.count() > 0));
		enableSingleAction(false);
	}
}

void KTNEFMain::openFile()
{
	QString	filename = KFileDialog::getOpenFileName(0,0,this,0);
	if (!filename.isEmpty()) loadFile(filename);
}

void KTNEFMain::viewFile()
{
	KTNEFAttach	*attach = view_->getSelection()->first();
	KURL		url("file:"+extractTemp(attach));
	QString		mimename(attach->mimeTag());

	if (mimename.isEmpty() || mimename == "application/octet-stream")
	{
		kdDebug() << "No mime type found in attachment object, trying to guess..." << endl;
		mimename = KMimeType::findByURL(url, 0, true)->name();
		kdDebug() << "Detected mime type: " << mimename << endl;
	}
	else
		kdDebug() << "Mime type from attachment object: " << mimename << endl;

	KRun::runURL(url, mimename);
}

QString KTNEFMain::extractTemp(KTNEFAttach *att)
{
	QString		dir = KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/";
	parser_->extractFileTo(att->name(), dir);
	dir.append(att->name());
	return dir;
}

void KTNEFMain::viewFileAs()
{
	KURL::List	list;
	list.append(extractTemp(view_->getSelection()->first()));

  KRun::displayOpenWithDialog(list);
//speleoalex KOpenWithHandler::getOpenWithHandler()->displayOpenWithDialog(list);
}

void KTNEFMain::extractFile()
{
	extractTo(defaultdir_);
}

void KTNEFMain::extractFileTo()
{
	QString	dir = KFileDialog::getExistingDirectory(lastdir_, this);
	if (!dir.isEmpty())
	{
		extractTo(dir);
		lastdir_ = dir;
	}
}

void KTNEFMain::extractAllFiles()
{
	QString	dir = KFileDialog::getExistingDirectory(lastdir_, this);
	if (!dir.isEmpty())
	{
		lastdir_ = dir;
		dir.append("/");
		QPtrList<KTNEFAttach>	list = parser_->message()->attachmentList();
		QPtrListIterator<KTNEFAttach>	it(list);
		for (;it.current();++it)
			if (!parser_->extractFileTo(it.current()->name(), dir))
			{
				QString	msg = i18n( "Unable to extract file \"%1\"" ).arg( it.current()->name() );
				QMessageBox::critical(this,i18n("Error"),msg,QMessageBox::Ok|QMessageBox::Default,0);
				return;
			}
	}
}

void KTNEFMain::propertiesFile()
{
	KTNEFAttach	*attach = view_->getSelection()->first();
	AttachPropertyDialog	dlg(this);
	dlg.setAttachment(attach);
	dlg.exec();
}

void KTNEFMain::optionDefaultDir()
{
	QString	dirname = KFileDialog::getExistingDirectory(defaultdir_, this);
	if (!dirname.isEmpty())
	{
		defaultdir_ = dirname;
		KGlobal::config()->setGroup("Settings");
		KGlobal::config()->writeEntry("defaultdir",defaultdir_);
	}
}

void KTNEFMain::viewSelectionChanged()
{
	QPtrList<KTNEFAttach>	*list = view_->getSelection();
	bool	on1 = (list->count() == 1u), on2 = (list->count() > 0u);
	actionCollection()->action("view_file")->setEnabled(on1);
	actionCollection()->action("view_file_as")->setEnabled(on1);
	actionCollection()->action("properties_file")->setEnabled(on1);

	actionCollection()->action("extract_file")->setEnabled(on2);
	actionCollection()->action("extract_file_to")->setEnabled(on2);
}

void KTNEFMain::enableExtractAll(bool on)
{
	if (!on) enableSingleAction(false);
	actionCollection()->action("extract_all_files")->setEnabled(on);
}

void KTNEFMain::enableSingleAction(bool on)
{
	actionCollection()->action("extract_file")->setEnabled(on);
	actionCollection()->action("extract_file_to")->setEnabled(on);
	actionCollection()->action("view_file")->setEnabled(on);
	actionCollection()->action("view_file_as")->setEnabled(on);
	actionCollection()->action("properties_file")->setEnabled(on);
}

void KTNEFMain::cleanup()
{
	QDir	d(KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/");
	const QFileInfoList	*list = d.entryInfoList(QDir::Files|QDir::Hidden,QDir::Unsorted);
	QFileInfoListIterator	it(*list);
	for (;it.current();++it)
		d.remove(it.current()->absFilePath());
}

void KTNEFMain::extractTo(const QString& dirname)
{
	QString	dir = dirname;
	if (dir.right(1) != "/") dir.append("/");
	QPtrList<KTNEFAttach>	*list = view_->getSelection();
	QPtrListIterator<KTNEFAttach>	it(*list);
	for (;it.current();++it)
		if (!parser_->extractFileTo(it.current()->name(), dir))
		{
			QString	msg = i18n("Unable to extract file \"%1\"").arg( it.current()->name() );
			QMessageBox::critical(this,i18n("Error"),msg,QMessageBox::Ok|QMessageBox::Default,0);
			return;
		}
}

/* This breaks the saveMainWindowSettings stuff....
  void KTNEFMain::closeEvent(QCloseEvent *e)
{
	e->accept();
}*/

void KTNEFMain::viewRightButtonPressed(QListViewItem*, const QPoint& p, int)
{
	QPtrList<KTNEFAttach>	*list = view_->getSelection();
	QPopupMenu m;
	if (list->count() > 0u)
	{
		if (list->count() == 1u)
		{
			m.insertItem(SmallIcon("viewmag"), i18n("View"), this, SLOT(viewFile()));
			m.insertItem(SmallIcon("package_applications"), i18n("View With..."), this, SLOT(viewFileAs()));
			m.insertSeparator();
		}
		m.insertItem(i18n("Extract"), this, SLOT(extractFile()));
		m.insertItem(SmallIcon("ktnef_extract_to"), i18n("Extract To..."), this, SLOT(extractFileTo()));
		if (list->count() == 1u)
		{
			m.insertSeparator();
			m.insertItem(SmallIcon("contents"), i18n("Properties..."), this, SLOT(propertiesFile()));
		}
	}
	else if ( list->count() == 0 )
		actionCollection()->action( "msg_properties" )->plug( &m );
	m.exec( p );
}

void KTNEFMain::viewDoubleClicked(QListViewItem *item)
{
	if (item && item->isSelected())
		viewFile();
}

void KTNEFMain::slotEditToolbars()
{
	saveMainWindowSettings( KGlobal::config(), QString::fromLatin1("MainWindow") );
	KEditToolbar	dlg(actionCollection());
	connect(&dlg, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
	dlg.exec();
}

void KTNEFMain::slotNewToolbarConfig()
{
	createGUI();
	applyMainWindowSettings( KGlobal::config(), QString::fromLatin1("MainWindow") );
}

void KTNEFMain::slotShowMessageProperties()
{
	MessagePropertyDialog dlg( this, parser_->message() );
	dlg.exec();
}

void KTNEFMain::slotShowMessageText()
{
	QString rtf = parser_->message()->rtfString();
	qDebug( "%s", rtf.latin1() );
	KTempFile tmpFile( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/", "rtf");
	*( tmpFile.textStream() ) << rtf;
	tmpFile.close();

	KRun::runURL( tmpFile.name(), "text/rtf", true );
}

void KTNEFMain::slotSaveMessageText()
{
	QString rtf = parser_->message()->rtfString();
	QString filename = KFileDialog::getSaveFileName( QString::null, QString::null, this );
	if ( !filename.isEmpty() )
	{
		QFile f( filename );
		if ( f.open( IO_WriteOnly ) )
		{
			QTextStream t( &f );
			t << rtf;
		}
		else
			QMessageBox::critical( this, i18n( "Error" ),
					i18n( "Unable to open file for writing, check file permissions." ),
					QMessageBox::Ok|QMessageBox::Default, 0);
	}
}

#include "ktnefmain.moc"

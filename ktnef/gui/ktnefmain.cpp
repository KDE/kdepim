/*
    ktnefmain.cpp

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <kdebug.h>
#include "ktnefmain.h"
#include <ktnef/ktnefparser.h>
#include "ktnefview.h"
#include <ktnef/ktnefattach.h>
#include <ktnef/ktnefproperty.h>
#include <ktnef/ktnefmessage.h>
#include "attachpropertydialog.h"
#include "messagepropertydialog.h"

#include <tqpopupmenu.h>
#include <klistview.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <tqpixmap.h>
#include <kstdaccel.h>
#include <tqmessagebox.h>
#include <kfiledialog.h>
#include <tqdir.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdialogbase.h>
#include <ktempfile.h>
#include <kkeydialog.h>

#ifdef KDE_NO_COMPAT
#undef KDE_NO_COMPAT
#endif

#include <krun.h>
#include <kopenwith.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kurldrag.h>


#define	NOT_IMPLEMENTED TQMessageBox::information(this, "ktnef", "Not implemented yet", TQMessageBox::Ok|TQMessageBox::Default, 0)

KTNEFMain::KTNEFMain(TQWidget *parent, const char *name)
	: KMainWindow(parent, name)
{
	setupActions();
	setupStatusbar();

	setupTNEF();

	KGlobal::config()->setGroup("Settings");
	defaultdir_ = KGlobal::config()->readPathEntry("defaultdir", "/tmp/");
	lastdir_ = defaultdir_;

	// create personale temo extract dir
	KStandardDirs::makeDir(KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp");

	resize(430,350);
	setAutoSaveSettings( "MainWindow" );
}

KTNEFMain::~KTNEFMain()
{
	delete parser_;
	cleanup();
}

void KTNEFMain::setupActions()
{
	// File menu
	KStdAction::open(this, TQT_SLOT(openFile()), actionCollection());
	KStdAction::quit(kapp, TQT_SLOT(quit()), actionCollection());

	// Action menu
	new KAction(i18n("View"), TQString("viewmag"), 0, this, TQT_SLOT(viewFile()), actionCollection(), "view_file");
	new KAction(i18n("View With..."), TQString("package_applications"), 0, this, TQT_SLOT(viewFileAs()), actionCollection(), "view_file_as");
	new KAction(i18n("Extract"), 0, this, TQT_SLOT(extractFile()), actionCollection(), "extract_file");
	new KAction(i18n("Extract To..."), TQString("ktnef_extract_to"), 0, this, TQT_SLOT(extractFileTo()), actionCollection(), "extract_file_to");
	new KAction(i18n("Extract All To..."), TQString("ktnef_extract_all_to"), 0, this, TQT_SLOT(extractAllFiles()), actionCollection(), "extract_all_files");
	new KAction( i18n( "Message Properties" ), "help", 0, this, TQT_SLOT( slotShowMessageProperties() ), actionCollection(), "msg_properties" );
	new KAction(i18n("Properties"), TQString("contents"), 0, this, TQT_SLOT(propertiesFile()), actionCollection(), "properties_file");
	new KAction( i18n( "Show Message Text" ), "mail_generic", 0, this, TQT_SLOT( slotShowMessageText() ), actionCollection(), "msg_text" );
	new KAction( i18n( "Save Message Text As..." ), "filesave", 0, this, TQT_SLOT( slotSaveMessageText() ), actionCollection(), "msg_save" );
	actionCollection()->action("view_file")->setEnabled(false);
	actionCollection()->action("view_file_as")->setEnabled(false);
	actionCollection()->action("extract_file")->setEnabled(false);
	actionCollection()->action("extract_file_to")->setEnabled(false);
	actionCollection()->action("extract_all_files")->setEnabled(false);
	actionCollection()->action("properties_file")->setEnabled(false);

	// Option menu
	new KAction(i18n("Default Folder..."), TQString("folder_open"), 0, this, TQT_SLOT(optionDefaultDir()), actionCollection(), "options_default_dir");

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	KStdAction::configureToolbars(this, TQT_SLOT(slotEditToolbars()), actionCollection());
        KStdAction::keyBindings( this, TQT_SLOT( slotConfigureKeys() ), actionCollection() );

	createGUI();
}

void KTNEFMain::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
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
	connect(view_, TQT_SIGNAL(selectionChanged()), TQT_SLOT(viewSelectionChanged()));
	connect(view_, TQT_SIGNAL(rightButtonPressed(TQListViewItem*,const TQPoint&,int)), TQT_SLOT(viewRightButtonPressed(TQListViewItem*,const TQPoint&,int)));
	connect(view_, TQT_SIGNAL(doubleClicked(TQListViewItem*)), TQT_SLOT(viewDoubleClicked(TQListViewItem*)));
	connect(view_, TQT_SIGNAL(dragRequested(const TQValueList<KTNEFAttach*>&)), TQT_SLOT(viewDragRequested(const TQValueList<KTNEFAttach*>&)));
}

void KTNEFMain::loadFile(const TQString& filename)
{
	filename_ = filename;
	setCaption(filename_);
	if (!parser_->openFile(filename))
	{
		TQMessageBox::critical(this, i18n("Error"), i18n("Unable to open file."), TQMessageBox::Ok|TQMessageBox::Default, 0);
		view_->setAttachments(0);
		enableExtractAll(false);
	}
	else
	{
		TQPtrList<KTNEFAttach>	list = parser_->message()->attachmentList();
		QString			msg;
		msg = i18n( "%n attachment found", "%n attachments found", list.count() );
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

TQString KTNEFMain::extractTemp(KTNEFAttach *att)
{
	QString		dir = KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/";
	parser_->extractFileTo(att->name(), dir);
	dir.append(att->name());
	return dir;
}

void KTNEFMain::viewFileAs()
{
	KURL::List	list;
	list.append(KURL::fromPathOrURL( extractTemp(view_->getSelection()->first()) ));

	KRun::displayOpenWithDialog(list);
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
		TQPtrList<KTNEFAttach>	list = parser_->message()->attachmentList();
		TQPtrListIterator<KTNEFAttach>	it(list);
		for (;it.current();++it)
			if (!parser_->extractFileTo(it.current()->name(), dir))
			{
				QString	msg = i18n( "Unable to extract file \"%1\"" ).arg( it.current()->name() );
				TQMessageBox::critical(this,i18n("Error"),msg,TQMessageBox::Ok|TQMessageBox::Default,0);
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
		KGlobal::config()->writePathEntry("defaultdir",defaultdir_);
	}
}

void KTNEFMain::viewSelectionChanged()
{
	TQPtrList<KTNEFAttach>	*list = view_->getSelection();
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
	const QFileInfoList	*list = d.entryInfoList(TQDir::Files|TQDir::Hidden,TQDir::Unsorted);
	QFileInfoListIterator	it(*list);
	for (;it.current();++it)
		d.remove(it.current()->absFilePath());
}

void KTNEFMain::extractTo(const TQString& dirname)
{
	QString	dir = dirname;
	if (dir.right(1) != "/") dir.append("/");
	TQPtrList<KTNEFAttach>	*list = view_->getSelection();
	TQPtrListIterator<KTNEFAttach>	it(*list);
	for (;it.current();++it)
		if (!parser_->extractFileTo(it.current()->name(), dir))
		{
			QString	msg = i18n("Unable to extract file \"%1\"").arg( it.current()->name() );
			TQMessageBox::critical(this,i18n("Error"),msg,TQMessageBox::Ok|TQMessageBox::Default,0);
			return;
		}
}

/* This breaks the saveMainWindowSettings stuff....
  void KTNEFMain::closeEvent(TQCloseEvent *e)
{
	e->accept();
}*/

void KTNEFMain::viewRightButtonPressed(TQListViewItem*, const TQPoint& p, int)
{
	TQPtrList<KTNEFAttach>	*list = view_->getSelection();
	TQPopupMenu m;
	if (list->count() > 0u)
	{
		if (list->count() == 1u)
		{
			m.insertItem(SmallIcon("viewmag"), i18n("View"), this, TQT_SLOT(viewFile()));
			m.insertItem(SmallIcon("package_applications"), i18n("View With..."), this, TQT_SLOT(viewFileAs()));
			m.insertSeparator();
		}
		m.insertItem(i18n("Extract"), this, TQT_SLOT(extractFile()));
		m.insertItem(SmallIcon("ktnef_extract_to"), i18n("Extract To..."), this, TQT_SLOT(extractFileTo()));
		if (list->count() == 1u)
		{
			m.insertSeparator();
			m.insertItem(SmallIcon("contents"), i18n("Properties"), this, TQT_SLOT(propertiesFile()));
		}
	}
	else if ( list->count() == 0 )
		actionCollection()->action( "msg_properties" )->plug( &m );
	m.exec( p );
}

void KTNEFMain::viewDoubleClicked(TQListViewItem *item)
{
	if (item && item->isSelected())
		viewFile();
}

void KTNEFMain::viewDragRequested( const TQValueList<KTNEFAttach*>& list )
{
	KURL::List urlList;
	for ( TQValueList<KTNEFAttach*>::ConstIterator it=list.constBegin(); it!=list.constEnd(); ++it )
		urlList << KURL( extractTemp( *it ) );
	if ( !list.isEmpty() )
	{
		KURLDrag *urlDrag = new KURLDrag( urlList, this );
		urlDrag->dragCopy();
	}
}

void KTNEFMain::slotEditToolbars()
{
	saveMainWindowSettings( KGlobal::config(), TQString::fromLatin1("MainWindow") );
	KEditToolbar	dlg(actionCollection());
	connect(&dlg, TQT_SIGNAL( newToolbarConfig() ), this, TQT_SLOT( slotNewToolbarConfig() ));
	dlg.exec();
}

void KTNEFMain::slotNewToolbarConfig()
{
	createGUI();
	applyMainWindowSettings( KGlobal::config(), TQString::fromLatin1("MainWindow") );
}

void KTNEFMain::slotShowMessageProperties()
{
	MessagePropertyDialog dlg( this, parser_->message() );
	dlg.exec();
}

void KTNEFMain::slotShowMessageText()
{
	TQString rtf = parser_->message()->rtfString();
	qDebug( "%s", rtf.latin1() );
	KTempFile tmpFile( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/", "rtf");
	*( tmpFile.textStream() ) << rtf;
	tmpFile.close();

	KRun::runURL( KURL::fromPathOrURL( tmpFile.name() ), "text/rtf", true );
}

void KTNEFMain::slotSaveMessageText()
{
	TQString rtf = parser_->message()->rtfString();
	TQString filename = KFileDialog::getSaveFileName( TQString::null, TQString::null, this );
	if ( !filename.isEmpty() )
	{
		TQFile f( filename );
		if ( f.open( IO_WriteOnly ) )
		{
			TQTextStream t( &f );
			t << rtf;
		}
		else
			TQMessageBox::critical( this, i18n( "Error" ),
					i18n( "Unable to open file for writing, check file permissions." ),
					TQMessageBox::Ok|TQMessageBox::Default, 0);
	}
}

#include "ktnefmain.moc"

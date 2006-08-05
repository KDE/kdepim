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
#include <ktnef/ktnefparser.h>
#include "ktnefview.h"
#include <ktnef/ktnefattach.h>
#include <ktnef/ktnefproperty.h>
#include <ktnef/ktnefmessage.h>
using namespace KTnef;
#include "ktnefmain.h"

#include "attachpropertydialog.h"
#include "messagepropertydialog.h"

#include <QMenu>
//Added by qt3to4:
#include <kicon.h>
#include <QTextStream>
#include <k3listview.h>
#include <klocale.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <QPixmap>
#include <kstdaccel.h>
#include <QMessageBox>
#include <kfiledialog.h>
#include <QDir>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdialog.h>
#include <ktempfile.h>
#include <kkeydialog.h>

#ifdef KDE_NO_COMPAT
#undef KDE_NO_COMPAT
#endif

#include <krun.h>
#include <kopenwith.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kurl.h>

#define	NOT_IMPLEMENTED QMessageBox::information(this, "ktnef", "Not implemented yet", QMessageBox::Ok|QMessageBox::Default, 0)

KTNEFMain::KTNEFMain( QWidget *parent )
	: KMainWindow( parent )
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
	KStdAction::open(this, SLOT(openFile()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	// Action menu
	KAction *action = new KAction(KIcon(QString("viewmag")), i18n("View"), actionCollection(), "view_file");
	connect(action, SIGNAL(triggered(bool)), SLOT(viewFile()));
	action = new KAction(KIcon(QString("package_applications")), i18n("View With..."), actionCollection(), "view_file_as");
	connect(action, SIGNAL(triggered(bool)), SLOT(viewFileAs()));
	action = new KAction(i18n("Extract"), actionCollection(), "extract_file");
	connect(action, SIGNAL(triggered(bool) ), SLOT(extractFile()));
	action = new KAction(KIcon(QString("ktnef_extract_to")), i18n("Extract To..."), actionCollection(), "extract_file_to");
	connect(action, SIGNAL(triggered(bool)), SLOT(extractFileTo()));
	action = new KAction(KIcon(QString("ktnef_extract_all_to")), i18n("Extract All To..."), actionCollection(), "extract_all_files");
	connect(action, SIGNAL(triggered(bool)), SLOT(extractAllFiles()));
	action = new KAction(KIcon("help"),  i18n( "Message Properties" ), actionCollection(), "msg_properties" );
	connect(action, SIGNAL(triggered(bool)), SLOT( slotShowMessageProperties() ));
	action = new KAction(KIcon(QString("contents")), i18n("Properties"), actionCollection(), "properties_file");
	connect(action, SIGNAL(triggered(bool)), SLOT(propertiesFile()));
	action = new KAction(KIcon("mail_generic"),  i18n( "Show Message Text" ), actionCollection(), "msg_text" );
	connect(action, SIGNAL(triggered(bool)), SLOT( slotShowMessageText() ));
	action = new KAction(KIcon("filesave"),  i18n( "Save Message Text As..." ), actionCollection(), "msg_save" );
	connect(action, SIGNAL(triggered(bool)), SLOT( slotSaveMessageText() ));
	actionCollection()->action("view_file")->setEnabled(false);
	actionCollection()->action("view_file_as")->setEnabled(false);
	actionCollection()->action("extract_file")->setEnabled(false);
	actionCollection()->action("extract_file_to")->setEnabled(false);
	actionCollection()->action("extract_all_files")->setEnabled(false);
	actionCollection()->action("properties_file")->setEnabled(false);

	// Option menu
	action = new KAction(KIcon(QString("folder_open")), i18n("Default Folder..."), actionCollection(), "options_default_dir");
	connect(action, SIGNAL(triggered(bool)), SLOT(optionDefaultDir()));

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	KStdAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());
        KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );

	createGUI();
}

void KTNEFMain::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), KKeyChooser::LetterShortcutsAllowed, this );
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
	connect(view_, SIGNAL(rightButtonPressed(Q3ListViewItem*,const QPoint&,int)), SLOT(viewRightButtonPressed(Q3ListViewItem*,const QPoint&,int)));
	connect(view_, SIGNAL(doubleClicked(Q3ListViewItem*)), SLOT(viewDoubleClicked(Q3ListViewItem*)));
	connect(view_, SIGNAL(dragRequested(const QList<KTNEFAttach*>&)), SLOT(viewDragRequested(const QList<KTNEFAttach*>&)));
}

void KTNEFMain::loadFile(const QString& filename)
{
	filename_ = filename;
	setCaption(filename_);
	if (!parser_->openFile(filename))
	{
		QMessageBox::critical(this, i18n("Error"), i18n("Unable to open file."), QMessageBox::Ok|QMessageBox::Default, 0);
		view_->setAttachments( QList<KTNEFAttach*>() );
		enableExtractAll(false);
	}
	else
	{
		QList<KTNEFAttach*>	list = parser_->message()->attachmentList();
		QString			msg;
		msg = i18np( "%n attachment found", "%n attachments found", list.count() );
		statusBar()->changeItem(msg, 0);
		view_->setAttachments(list);
		enableExtractAll((list.count() > 0));
		enableSingleAction(false);
	}
}

void KTNEFMain::openFile()
{
	QString	filename = KFileDialog::getOpenFileName(KUrl(),0,this,0);
	if (!filename.isEmpty()) loadFile(filename);
}

void KTNEFMain::viewFile()
{
	KTNEFAttach	*attach = view_->getSelection()->first();
	KUrl		url("file:"+extractTemp(attach));
	QString		mimename(attach->mimeTag());

	if (mimename.isEmpty() || mimename == "application/octet-stream")
	{
		kDebug() << "No mime type found in attachment object, trying to guess..." << endl;
		mimename = KMimeType::findByURL(url, 0, true)->name();
		kDebug() << "Detected mime type: " << mimename << endl;
	}
	else
		kDebug() << "Mime type from attachment object: " << mimename << endl;

	KRun::runUrl(url, mimename, this);
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
	KUrl::List	list;
	list.append( KUrl( extractTemp(view_->getSelection()->first()) ));

	KRun::displayOpenWithDialog(list, this);
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
		QList<KTNEFAttach*> list = parser_->message()->attachmentList();
		KTNEFAttach *it;
		Q_FOREACH( it, list )
		{
			if (!parser_->extractFileTo(it->name(), dir))
			{
				QString	msg = i18n( "Unable to extract file \"%1\"", it->name() );
				QMessageBox::critical(this,i18n("Error"),msg,QMessageBox::Ok|QMessageBox::Default,0);
				return;
			}
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
	QList<KTNEFAttach*>	*list = view_->getSelection();
	bool	on1 = (list->count() == 1), on2 = (list->count() > 0);
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
	const QFileInfoList	list = d.entryInfoList(QDir::Files|QDir::Hidden,QDir::Unsorted);
	for (int i = 0; i < list.size(); ++i)
	{
			d.remove(list.at(i).absoluteFilePath());
	}
}

void KTNEFMain::extractTo(const QString& dirname)
{
	QString	dir = dirname;
	if (dir.right(1) != "/") dir.append("/");
	QList<KTNEFAttach*>	*list = view_->getSelection();
	QListIterator<KTNEFAttach*>	it(*list);
	while (it.hasNext())
		if (!parser_->extractFileTo(it.next()->name(), dir))
		{
			QString	msg = i18n("Unable to extract file \"%1\"", it.next()->name() );
			QMessageBox::critical(this,i18n("Error"),msg,QMessageBox::Ok|QMessageBox::Default,0);
			return;
		}
}

/* This breaks the saveMainWindowSettings stuff....
  void KTNEFMain::closeEvent(QCloseEvent *e)
{
	e->accept();
}*/

void KTNEFMain::viewRightButtonPressed(Q3ListViewItem*, const QPoint& p, int)
{
	QList<KTNEFAttach*>	*list = view_->getSelection();
	QMenu m;
	if (list->count() > 0)
	{
		if (list->count() == 1)
		{
			m.addAction(SmallIcon("viewmag"), i18n("View"), this, SLOT(viewFile()));
			m.addAction(SmallIcon("package_applications"), i18n("View With..."), this, SLOT(viewFileAs()));
			m.addSeparator();
		}
		m.addAction(i18n("Extract"), this, SLOT(extractFile()));
		m.addAction(SmallIcon("ktnef_extract_to"), i18n("Extract To..."), this, SLOT(extractFileTo()));
		if (list->count() == 1u)
		{
			m.addSeparator();
			m.addAction(SmallIcon("contents"), i18n("Properties"), this, SLOT(propertiesFile()));
		}
	}
	else if ( list->count() == 0 )
		m.addAction( actionCollection()->action( "msg_properties" ) );
	m.exec( p );
}

void KTNEFMain::viewDoubleClicked(Q3ListViewItem *item)
{
	if (item && item->isSelected())
		viewFile();
}

void KTNEFMain::viewDragRequested( const QList<KTNEFAttach*>& list )
{
	KUrl::List urlList;
	for ( QList<KTNEFAttach*>::ConstIterator it=list.constBegin(); it!=list.constEnd(); ++it )
		urlList << KUrl( extractTemp( *it ) );
	if ( !list.isEmpty() )
	{
#warning Port KURLDrag usage
//                KURLDrag *urlDrag = new KURLDrag( urlList, this );
//                urlDrag->dragCopy();
	}
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
	qDebug( "%s", rtf.toLatin1().data() );
	KTempFile tmpFile( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/", "rtf");
	*( tmpFile.textStream() ) << rtf;
	tmpFile.close();

	KRun::runUrl( KUrl( tmpFile.name() ), "text/rtf", this, true );
}

void KTNEFMain::slotSaveMessageText()
{
	QString rtf = parser_->message()->rtfString();
	QString filename = KFileDialog::getSaveFileName( QString(), QString(), this );
	if ( !filename.isEmpty() )
	{
		QFile f( filename );
		if ( f.open( QIODevice::WriteOnly ) )
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

/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "ktnefmain.h"
#include "attachpropertydialog.h"
#include "ktnefview.h"
#include "messagepropertydialog.h"

#include <KTNEF/KTNEFAttach>
#include <KTNEF/KTNEFMessage>
#include <KTNEF/KTNEFParser>
#include <KTNEF/KTNEFProperty>

#include <KAction>
#include <KActionCollection>
#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KEditToolBar>
#include <KFileDialog>
#include <KGlobal>
#include <KIconLoader>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KProcess>
#include <KRun>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KStandardDirs>
#include <KStatusBar>
#include <KTemporaryFile>

#include <QContextMenuEvent>
#include <QDir>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>

KTNEFMain::KTNEFMain( QWidget *parent )
  : KXmlGuiWindow( parent )
{
  setupActions();
  setupStatusbar();

  setupTNEF();

  KConfigGroup config( KGlobal::config(), "Settings" );
  mDefaultDir = config.readPathEntry( "defaultdir", "/tmp/" );
  mLastDir = mDefaultDir;

  // create personale temo extract dir
  KStandardDirs::makeDir( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp" );

  resize( 430, 350 );

  setStandardToolBarMenuEnabled( true );

  createStandardStatusBarAction();

  setupGUI( Keys | Save | Create, "ktnefui.rc" );

  setAutoSaveSettings();
}

KTNEFMain::~KTNEFMain()
{
  delete mParser;
  cleanup();
}

void KTNEFMain::setupActions()
{
  KStandardAction::quit( this, SLOT(close()), actionCollection() );

  KAction *action =
    KStandardAction::keyBindings( this, SLOT(slotConfigureKeys()), actionCollection() );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can configure "
           "the application-wide shortcuts." ) );

  KStandardAction::configureToolbars( this, SLOT(slotEditToolbars()), actionCollection() );

  // File menu
  KStandardAction::open( this, SLOT(openFile()), actionCollection() );

  // Action menu
  KAction *openAction = actionCollection()->addAction( "view_file" );
  openAction->setText( i18nc( "@action:inmenu", "View" ) );
  openAction->setIcon( KIcon( "document-open" ) );
  connect( openAction, SIGNAL(triggered()), this, SLOT(viewFile()) );

  KAction *openAsAction = actionCollection()->addAction( "view_file_as" );
  openAsAction->setText( i18nc( "@action:inmenu", "View With..." ) );
  connect( openAsAction, SIGNAL(triggered()), this, SLOT(viewFileAs()) );

  KAction *extractAction = actionCollection()->addAction( "extract_file" );
  extractAction->setText( i18nc( "@action:inmenu", "Extract" ) );
  connect( extractAction, SIGNAL(triggered()), this, SLOT(extractFile()) );

  KAction *extractToAction = actionCollection()->addAction( "extract_file_to" );
  extractToAction->setText( i18nc( "@action:inmenu", "Extract To..." ) );
  extractToAction->setIcon( KIcon( "archive-extract" ) );
  connect( extractToAction, SIGNAL(triggered()), this, SLOT(extractFileTo()) );

  KAction *extractAllToAction = actionCollection()->addAction( "extract_all_files" );
  extractAllToAction->setText( i18nc( "@action:inmenu", "Extract All To..." ) );
  extractAllToAction->setIcon( KIcon( "archive-extract" ) );
  connect( extractAllToAction, SIGNAL(triggered()), this, SLOT(extractAllFiles()) );

  KAction *filePropsAction = actionCollection()->addAction( "properties_file" );
  filePropsAction->setText( i18nc( "@action:inmenu", "Properties" ) );
  filePropsAction->setIcon( KIcon( "document-properties" ) );
  connect( filePropsAction, SIGNAL(triggered()), this, SLOT(propertiesFile()));

  KAction *messPropsAction = actionCollection()->addAction( "msg_properties" );
  messPropsAction->setText( i18nc( "@action:inmenu", "Message Properties" ) );
  connect( messPropsAction, SIGNAL(triggered()), this, SLOT(slotShowMessageProperties()) );

  KAction *messShowAction = actionCollection()->addAction( "msg_text" );
  messShowAction->setText( i18nc( "@action:inmenu", "Show Message Text" ) );
  messShowAction->setIcon( KIcon( "document-preview-archive" ) );
  connect( messShowAction, SIGNAL(triggered()), this, SLOT(slotShowMessageText()) );

  KAction *messSaveAction = actionCollection()->addAction( "msg_save" );
  messSaveAction->setText( i18nc( "@action:inmenu", "Save Message Text As..." ) );
  messSaveAction->setIcon( KIcon( "document-save" ) );
  connect( messSaveAction, SIGNAL(triggered()), this, SLOT(slotSaveMessageText()) );

  actionCollection()->action( "view_file" )->setEnabled( false );
  actionCollection()->action( "view_file_as" )->setEnabled( false );
  actionCollection()->action( "extract_file" )->setEnabled( false );
  actionCollection()->action( "extract_file_to" )->setEnabled( false );
  actionCollection()->action( "extract_all_files" )->setEnabled( false );
  actionCollection()->action( "properties_file" )->setEnabled( false );

  // Options menu
  KAction *defFolderAction = actionCollection()->addAction( "options_default_dir" );
  defFolderAction->setText( i18nc( "@action:inmenu", "Default Folder..." ) );
  defFolderAction->setIcon( KIcon( "folder-open" ) );
  connect( defFolderAction, SIGNAL(triggered()), this, SLOT(optionDefaultDir()) );

}

void KTNEFMain::slotConfigureKeys()
{
  KShortcutsDialog::configure( actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this );
}

void KTNEFMain::setupStatusbar()
{
  statusBar()->insertItem( i18nc( "@info:status", "100 attachments found" ), 0 );
  statusBar()->changeItem( i18nc( "@info:status", "No file loaded" ), 0 );
}

void KTNEFMain::setupTNEF()
{
  mView = new KTNEFView( this );
  mView->setAllColumnsShowFocus( true );
  mParser = new KTNEFParser;

  setCentralWidget(mView);

  connect( mView, SIGNAL(itemSelectionChanged()),
           SLOT(viewSelectionChanged()) );

  connect( mView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(viewDoubleClicked(QTreeWidgetItem*)) );

//PORTME:  connect( mView, SIGNAL(dragRequested(QList<KTNEFAttach*>)),
//PORTME:           SLOT(viewDragRequested(QList<KTNEFAttach*>)) );
}

void KTNEFMain::loadFile( const QString &filename )
{
  mFilename = filename;
  setCaption(mFilename);
  if ( !mParser->openFile( filename ) ) {
    KMessageBox::error(
      this,
      i18nc( "@info",
             "Unable to open file \"%1\".", filename ) );
    mView->setAttachments( QList<KTNEFAttach *>() );
    enableExtractAll( false );
  } else {
    QList<KTNEFAttach *> list = mParser->message()->attachmentList();
    QString msg;
    msg = i18ncp( "@info:status",
                  "%1 attachment found", "%1 attachments found", list.count() );
    statusBar()->changeItem( msg, 0 );
    mView->setAttachments( list );
    enableExtractAll( ( list.count() > 0 ) );
    enableSingleAction( false );
  }
}

void KTNEFMain::openFile()
{
  QString filename =
    KFileDialog::getOpenFileName(
      KUrl(),
      QString(),
      this,
      i18nc( "@title:window", "Open TNEF File" ) );
  if ( !filename.isEmpty() ) {
    loadFile( filename );
  }
}

void KTNEFMain::viewFile()
{
  if ( !mView->getSelection().isEmpty() ) {
    KTNEFAttach *attach = mView->getSelection().first();
    KUrl url( "file:" + extractTemp( attach ) );
    QString mimename( attach->mimeTag() );

    if ( mimename.isEmpty() || mimename == QLatin1String("application/octet-stream") ) {
      kDebug() << "No mime type found in attachment object, trying to guess...";
      mimename = KMimeType::findByUrl( url, 0, true )->name();
      kDebug() << "Detected mime type: " << mimename;
    } else {
      kDebug() << "Mime type from attachment object: " << mimename;
    }

    KRun::runUrl( url, mimename, this, true );
  } else {
    KMessageBox::information(
      this,
      i18nc( "@info",
             "There is no file selected. Please select a file an try again." ) );
  }

}

QString KTNEFMain::extractTemp( KTNEFAttach *att )
{
  QString dir = KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/";
  mParser->extractFileTo( att->name(), dir );
  dir.append( att->name() );
  return dir;
}

void KTNEFMain::viewFileAs()
{
  if ( !mView->getSelection().isEmpty() ) {
    KUrl::List list;
    list.append( KUrl( extractTemp( mView->getSelection().first() ) ) );

    if ( !list.isEmpty() ) {
      KRun::displayOpenWithDialog( list, this );
    }
  } else {
    KMessageBox::information(
      this,
      i18nc( "@info",
             "There is no file selected. Please select a file an try again." ) );
  }
}

void KTNEFMain::extractFile()
{
  extractTo( mDefaultDir );
}

void KTNEFMain::extractFileTo()
{
  QString dir = KFileDialog::getExistingDirectory( mLastDir, this );
  if ( !dir.isEmpty() ) {
    extractTo( dir );
    mLastDir = dir;
  }
}

void KTNEFMain::extractAllFiles()
{
  QString dir = KFileDialog::getExistingDirectory( mLastDir, this );
  if ( !dir.isEmpty() ) {
    mLastDir = dir;
    dir.append( "/" );
    QList<KTNEFAttach *> list = mParser->message()->attachmentList();
    QList<KTNEFAttach *>::ConstIterator it;
    QList<KTNEFAttach *>::ConstIterator end( list.constEnd() );
    for ( it = list.constBegin(); it != end; ++it ) {
      if ( !mParser->extractFileTo( (*it)->name(), dir ) ) {
        KMessageBox::error(
          this,
          i18nc( "@info",
                 "Unable to extract file \"%1\".", (*it)->name() ) );
        return;
      }
    }
  }
}

void KTNEFMain::propertiesFile()
{
  KTNEFAttach *attach = mView->getSelection().first();
  AttachPropertyDialog dlg( this );
  dlg.setAttachment( attach );
  dlg.exec();
}

void KTNEFMain::optionDefaultDir()
{
  const QString dirname = KFileDialog::getExistingDirectory( mDefaultDir, this );
  if ( !dirname.isEmpty() ) {
    mDefaultDir = dirname;

    KConfigGroup config( KGlobal::config(), "Settings" );
    config.writePathEntry( "defaultdir", mDefaultDir );
  }
}

void KTNEFMain::viewSelectionChanged()
{
  const QList<KTNEFAttach *> list = mView->getSelection();
  bool on1 = ( list.count() == 1 );
  bool on2 = ( list.count() > 0 );

  actionCollection()->action( "view_file" )->setEnabled( on1 );
  actionCollection()->action( "view_file_as" )->setEnabled( on1 );
  actionCollection()->action( "properties_file" )->setEnabled( on1 );

  actionCollection()->action( "extract_file" )->setEnabled( on2 );
  actionCollection()->action( "extract_file_to" )->setEnabled( on2 );
}

void KTNEFMain::enableExtractAll( bool on )
{
  if ( !on ) {
    enableSingleAction( false );
  }

  actionCollection()->action( "extract_all_files" )->setEnabled( on );

}

void KTNEFMain::enableSingleAction( bool on )
{
  actionCollection()->action( "extract_file" )->setEnabled( on );
  actionCollection()->action( "extract_file_to" )->setEnabled( on );
  actionCollection()->action( "view_file" )->setEnabled( on );
  actionCollection()->action( "view_file_as" )->setEnabled( on );
  actionCollection()->action( "properties_file" )->setEnabled( on );
}

void KTNEFMain::cleanup()
{
  QDir d( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/" );
  QFileInfoList list = d.entryInfoList( QDir::Files | QDir::Hidden, QDir::Unsorted );
  QFileInfoList::iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    d.remove( it->absoluteFilePath() );
  }
}

void KTNEFMain::extractTo( const QString &dirname )
{
  QString dir = dirname;
  if ( dir.right(1) != "/" ) {
    dir.append( "/" );
  }
  QList<KTNEFAttach *>list = mView->getSelection();
  QList<KTNEFAttach *>::ConstIterator it;
  QList<KTNEFAttach *>::ConstIterator end( list.constEnd() );
  for ( it = list.constBegin(); it != end; ++it ) {
    if ( !mParser->extractFileTo( (*it)->name(), dir ) ) {
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Unable to extract file \"%1\".", (*it)->name() ) );
      return;
    }
  }
}

/* This breaks the saveMainWindowSettings stuff....
  void KTNEFMain::closeEvent(QCloseEvent *e)
{
  e->accept();
}*/

void KTNEFMain::contextMenuEvent( QContextMenuEvent *event )
{
  QList<KTNEFAttach *> list = mView->getSelection();
  if ( !list.count() ) {
    return;
  }

  QAction *view = 0;
  QAction *viewWith = 0;
  QAction *prop = 0;
  KMenu *menu = new KMenu();
  if ( list.count() == 1 ) {
    view = menu->addAction( KIcon( "document-open" ),
                            i18nc( "@action:inmenu", "View" ) );
    viewWith = menu->addAction( i18nc( "@action:inmenu", "View With..." ) );
    menu->addSeparator();
  }
  QAction *extract = menu->addAction( i18nc( "@action:inmenu", "Extract" ) );
  QAction *extractTo = menu->addAction( KIcon( "archive-extract" ),
                                        i18nc( "@action:inmenu", "Extract To..." ) );
  if ( list.count() == 1 ) {
    menu->addSeparator();
   prop = menu->addAction( KIcon( "document-properties" ),
                           i18nc( "@action:inmenu", "Properties" ) );
  }

  QAction *a = menu->exec( event->globalPos(), 0 );
  if ( a ) {
    if ( a == view ) {
      viewFile();
    } else if ( a == viewWith ) {
      viewFileAs();
    } else if ( a == extract ) {
      extractFile();
    } else if ( a == extractTo ) {
      extractFileTo();
    } else if ( a == prop ) {
      propertiesFile();
    }
  }
  delete menu;
}

void KTNEFMain::viewDoubleClicked( QTreeWidgetItem *item )
{
  if ( item && item->isSelected() ) {
    viewFile();
  }
}

void KTNEFMain::viewDragRequested( const QList<KTNEFAttach *>& list )
{
  KUrl::List urlList;
  QList<KTNEFAttach *>::ConstIterator end( list.constEnd() );
  for ( QList<KTNEFAttach *>::ConstIterator it = list.constBegin();
        it != end; ++it ) {
    urlList << KUrl( extractTemp( *it ) );
  }

  if ( !list.isEmpty() ) {
    QMimeData *mimeData = new QMimeData;
    urlList.populateMimeData( mimeData );

    QDrag *drag = new QDrag( this );
    drag->setMimeData( mimeData );
  }
}

void KTNEFMain::slotEditToolbars()
{
  saveMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );

  KEditToolBar dlg( factory() );
  connect( &dlg, SIGNAL(newToolBarConfig()), this, SLOT(newToolbarConfig()) );
  dlg.exec();
}

void KTNEFMain::slotNewToolbarConfig()
{
  createGUI( "ktnefui.rc" );
  applyMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );
}

void KTNEFMain::slotShowMessageProperties()
{
  MessagePropertyDialog dlg( this, mParser->message() );
  dlg.exec();
}

void KTNEFMain::slotShowMessageText()
{
  if ( !mParser->message() ) {
    return;
  }

  QString rtf = mParser->message()->rtfString();
  if ( !rtf.isEmpty() ) {
    KTemporaryFile *tmpFile = new KTemporaryFile();
    tmpFile->setPrefix( KGlobal::dirs()->localkdedir() + "/share/apps/ktnef/tmp/" );
    tmpFile->setSuffix( QLatin1String( ".rtf" ) );
    tmpFile->open();
    tmpFile->setPermissions( QFile::ReadUser );
    tmpFile->write( rtf.toLocal8Bit() );
    tmpFile->close();

    KRun::runUrl( KUrl( tmpFile->fileName() ), "text/rtf", this, true );
    delete tmpFile;
  } else {
      KMessageBox::error(
        this,
        i18nc( "@info",
               "The message does not contain any Rich Text data." ) );
  }
}

void KTNEFMain::slotSaveMessageText()
{
  if ( !mParser->message() ) {
    return;
  }

  QString rtf = mParser->message()->rtfString();
  QString filename = KFileDialog::getSaveFileName( QString(), QString(), this );
  if ( !filename.isEmpty() ) {
    QFile f( filename );
    if ( f.open( QIODevice::WriteOnly ) ) {
      QTextStream t( &f );
      t << rtf;
    } else {
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Unable to open file \"%1\" for writing, check file permissions.", filename ) );
    }
  }
}

#include "ktnefmain.moc"

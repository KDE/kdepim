/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <unistd.h>

#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqpixmap.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>
#include <tqgroupbox.h>
#include <tqwidgetfactory.h>
#include <tqregexp.h>
#include <tqtimer.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klistview.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kactivelabel.h>
#include <kdirwatch.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kio/netaccess.h>

#include "kcmdesignerfields.h"

using namespace KPIM;

class PageItem : public QCheckListItem
{
  public:
    PageItem( TQListView *parent, const TQString &path )
      : TQCheckListItem( parent, "", TQCheckListItem::CheckBox ),
        mPath( path ), mIsActive( false )
    {
      mName = path.mid( path.findRev( '/' ) + 1 );

      TQWidget *wdg = TQWidgetFactory::create( mPath, 0, 0 );
      if ( wdg ) {
        setText( 0, wdg->caption() );

        TQPixmap pm = TQPixmap::grabWidget( wdg );
        TQImage img = pm.convertToImage().smoothScale( 300, 300, TQImage::ScaleMin );
        mPreview = img;

        TQObjectList *list = wdg->queryList( "TQWidget" );
        TQObjectListIt it( *list );

        TQMap<TQString, TQString> allowedTypes;
        allowedTypes.insert( "TQLineEdit", i18n( "Text" ) );
        allowedTypes.insert( "TQTextEdit", i18n( "Text" ) );
        allowedTypes.insert( "TQSpinBox", i18n( "Numeric Value" ) );
        allowedTypes.insert( "TQCheckBox", i18n( "Boolean" ) );
        allowedTypes.insert( "TQComboBox", i18n( "Selection" ) );
        allowedTypes.insert( "QDateTimeEdit", i18n( "Date & Time" ) );
        allowedTypes.insert( "KLineEdit", i18n( "Text" ) );
        allowedTypes.insert( "KDateTimeWidget", i18n( "Date & Time" ) );
        allowedTypes.insert( "KDatePicker", i18n( "Date" ) );

        while ( it.current() ) {
          if ( allowedTypes.find( it.current()->className() ) != allowedTypes.end() ) {
            TQString name = it.current()->name();
            if ( name.startsWith( "X_" ) ) {
              new TQListViewItem( this, name,
                                 allowedTypes[ it.current()->className() ],
                                 it.current()->className(),
                                 TQWhatsThis::textFor( static_cast<TQWidget*>( it.current() ) ) );
            }
          }

          ++it;
        }

        delete list;
      } 
    }

    TQString name() const { return mName; }
    TQString path() const { return mPath; }

    TQPixmap preview()
    {
      return mPreview;
    }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  protected:
    void paintBranches( TQPainter *p, const TQColorGroup & cg, int w, int y, int h )
    {
      TQListViewItem::paintBranches( p, cg, w, y, h );
    }

  private:
    TQString mName;
    TQString mPath;
    TQPixmap mPreview;
    bool mIsActive;
};

KCMDesignerFields::KCMDesignerFields( TQWidget *parent, const char *name )
  : KCModule( parent, name )
{
  TQTimer::singleShot( 0, this, TQT_SLOT( delayedInit() ) );
  
  KAboutData *about = new KAboutData( I18N_NOOP( "KCMDesignerfields" ),
                                      I18N_NOOP( "Qt Designer Fields Dialog" ),
                                      0, 0, KAboutData::License_LGPL,
                                      I18N_NOOP( "(c), 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  setAboutData( about );
}

void KCMDesignerFields::delayedInit()
{
  kdDebug() << "KCMDesignerFields::delayedInit()" << endl;

  initGUI();

  connect( mPageView, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
           this, TQT_SLOT( updatePreview( TQListViewItem* ) ) );
  connect( mPageView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           this, TQT_SLOT( itemClicked( TQListViewItem* ) ) );

  connect( mDeleteButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( deleteFile() ) );
  connect( mImportButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( importFile() ) );
  connect( mDesignerButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( startDesigner() ) );

  load();

  // Install a dirwatcher that will detect newly created or removed designer files
  KDirWatch *dw = new KDirWatch( this );
  KStandardDirs::makeDir(localUiDir());
  dw->addDir( localUiDir(), true );
  connect( dw, TQT_SIGNAL( created(const TQString&) ), TQT_SLOT( rebuildList() ) );
  connect( dw, TQT_SIGNAL( deleted(const TQString&) ), TQT_SLOT( rebuildList() ) );
  connect( dw, TQT_SIGNAL( dirty(const TQString&) ),   TQT_SLOT( rebuildList() ) );
}

void KCMDesignerFields::deleteFile()
{
  TQListViewItem *item = mPageView->selectedItem();
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item->parent() ? item->parent() : item );
    if (KMessageBox::warningContinueCancel(this,
	i18n( "<qt>Do you really want to delete '<b>%1</b>'?</qt>").arg( pageItem->text(0) ), "", KStdGuiItem::del() )
         == KMessageBox::Continue)
      KIO::NetAccess::del( pageItem->path(), 0 );
  }
  // The actual view refresh will be done automagically by the slots connected to kdirwatch
}

void KCMDesignerFields::importFile()
{
  KURL src = KFileDialog::getOpenFileName( TQDir::homeDirPath(), i18n("*.ui|Designer Files"),
                                              this, i18n("Import Page") );
  KURL dest = localUiDir();
  dest.setFileName(src.fileName());
  KIO::NetAccess::file_copy( src, dest, -1, true, false, this );
  // The actual view refresh will be done automagically by the slots connected to kdirwatch
}


void KCMDesignerFields::loadUiFiles()
{
  TQStringList list = KGlobal::dirs()->findAllResources( "data", uiPath() + "/*.ui", true, true );
  for ( TQStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    new PageItem( mPageView, *it );
  }
}

void KCMDesignerFields::rebuildList()
{
  TQStringList ai = saveActivePages();
  updatePreview( 0 );
  mPageView->clear();
  loadUiFiles();
  loadActivePages(ai);
}

void KCMDesignerFields::loadActivePages(const TQStringList& ai)
{
  TQListViewItemIterator it( mPageView );
  while ( it.current() ) {
    if ( it.current()->parent() == 0 ) {
      PageItem *item = static_cast<PageItem*>( it.current() );
      if ( ai.find( item->name() ) != ai.end() ) {
        item->setOn( true );
        item->setIsActive( true );
      }
    }

    ++it;
  }
}

void KCMDesignerFields::load()
{
  loadActivePages( readActivePages() );
}

TQStringList KCMDesignerFields::saveActivePages()
{
  TQListViewItemIterator it( mPageView, TQListViewItemIterator::Checked |
                            TQListViewItemIterator::Selectable );

  TQStringList activePages;
  while ( it.current() ) {
    if ( it.current()->parent() == 0 ) {
      PageItem *item = static_cast<PageItem*>( it.current() );
      activePages.append( item->name() );
    }

    ++it;
  }

  return activePages;
}

void KCMDesignerFields::save()
{
  writeActivePages( saveActivePages() );
}

void KCMDesignerFields::defaults()
{
}

void KCMDesignerFields::initGUI()
{
  TQVBoxLayout *layout = new TQVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  bool noDesigner = KStandardDirs::findExe("designer").isEmpty();

  if ( noDesigner )
  {
    TQString txt =
      i18n("<qt><b>Warning:</b> Qt Designer could not be found. It is probably not "
         "installed. You will only be able to import existing designer files.</qt>");
    TQLabel *lbl = new TQLabel( txt, this );
    layout->addWidget( lbl );
  }

  TQHBoxLayout *hbox = new TQHBoxLayout( layout, KDialog::spacingHint() );

  mPageView = new KListView( this );
  mPageView->addColumn( i18n( "Available Pages" ) );
  mPageView->setRootIsDecorated( true );
  mPageView->setAllColumnsShowFocus( true );
  mPageView->setFullWidth( true );
  hbox->addWidget( mPageView );

  TQGroupBox *box = new TQGroupBox(1, Qt::Horizontal, i18n("Preview of Selected Page"), this );

  mPagePreview = new TQLabel( box );
  mPagePreview->setMinimumWidth( 300 );

  mPageDetails = new TQLabel( box );

  hbox->addWidget( box );

  loadUiFiles();

  hbox = new TQHBoxLayout( layout, KDialog::spacingHint() );

  TQString cwHowto = i18n("<qt><p>This section allows you to add your own GUI"
                         "  Elements ('<i>Widgets</i>') to store your own values"
                         " into %1. Proceed as described below:</p>"
                         "<ol>"
                         "<li>Click on '<i>Edit with Qt Designer</i>'"
                         "<li>In the dialog, select '<i>Widget</i>', then click <i>OK</i>"
                         "<li>Add your widgets to the form"
                         "<li>Save the file in the directory proposed by Qt Designer"
                         "<li>Close Qt Designer"
                         "</ol>"
                         "<p>In case you already have a designer file (*.ui) located"
                         " somewhere on your hard disk, simply choose '<i>Import Page</i>'</p>"
                         "<p><b>Important:</b> The name of each input widget you place within"
                         " the form must start with '<i>X_</i>'; so if you want the widget to"
                         " correspond to your custom entry '<i>X-Foo</i>', set the widget's"
                         " <i>name</i> property to '<i>X_Foo</i>'.</p>"
                         "<p><b>Important:</b> The widget will edit custom fields with an"
                         " application name of %2.  To change the application name"
                         " to be edited, set the widget name in Qt Designer.</p></qt>" )
                         .arg( applicationName(), applicationName() );

  KActiveLabel *activeLabel = new KActiveLabel(
      i18n( "<a href=\"whatsthis:%1\">How does this work?</a>" ).arg(cwHowto), this );
  hbox->addWidget( activeLabel );

  // ### why is this needed? Looks like a KActiveLabel bug...
  activeLabel->setSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Maximum );

  hbox->addStretch( 1 );

  mDeleteButton = new TQPushButton( i18n( "Delete Page" ), this);
  mDeleteButton->setEnabled( false );
  hbox->addWidget( mDeleteButton );
  mImportButton = new TQPushButton( i18n( "Import Page..." ), this);
  hbox->addWidget( mImportButton );
  mDesignerButton = new TQPushButton( i18n( "Edit with Qt Designer..." ), this );
  hbox->addWidget( mDesignerButton );

  if ( noDesigner )
    mDesignerButton->setEnabled( false );

  // FIXME: Why do I have to call show() for all widgets? A this->show() doesn't
  // seem to work.
  mPageView->show();
  box->show();
  activeLabel->show();
  mDeleteButton->show();
  mImportButton->show();
  mDesignerButton->show();
}

void KCMDesignerFields::updatePreview( TQListViewItem *item )
{
  bool widgetItemSelected = false;

  if ( item ) {
    if ( item->parent() ) {
      TQString details = TQString( "<qt><table>"
                                 "<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>"
                                 "<tr><td align=\"right\"><b>%3</b></td><td>%4</td></tr>"
                                 "<tr><td align=\"right\"><b>%5</b></td><td>%6</td></tr>"
                                 "<tr><td align=\"right\"><b>%7</b></td><td>%8</td></tr>"
                                 "</table></qt>" )
                                .arg( i18n( "Key:" ) )
                                .arg( item->text( 0 ).replace("X_","X-") )
                                .arg( i18n( "Type:" ) )
                                .arg( item->text( 1 ) )
                                .arg( i18n( "Classname:" ) )
                                .arg( item->text( 2 ) )
                                .arg( i18n( "Description:" ) )
                                .arg( item->text( 3 ) );

      mPageDetails->setText( details );

      PageItem *pageItem = static_cast<PageItem*>( item->parent() );
      mPagePreview->setPixmap( pageItem->preview() );
    } else {
      mPageDetails->setText( TQString::null );

      PageItem *pageItem = static_cast<PageItem*>( item );
      mPagePreview->setPixmap( pageItem->preview() );

      widgetItemSelected = true;
    }

    mPagePreview->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
  } else {
    mPagePreview->setPixmap( TQPixmap() );
    mPagePreview->setFrameStyle( 0 );
    mPageDetails->setText( TQString::null );
  }

  mDeleteButton->setEnabled( widgetItemSelected );
}

void KCMDesignerFields::itemClicked( TQListViewItem *item )
{
  if ( !item || item->parent() != 0 )
    return;

  PageItem *pageItem = static_cast<PageItem*>( item );

  if ( pageItem->isOn() != pageItem->isActive() ) {
    emit changed( true );
    pageItem->setIsActive( pageItem->isOn() );
  }
}

void KCMDesignerFields::startDesigner()
{
  TQString cmdLine = "designer";

  // check if path exists and create one if not.
  TQString cepPath = localUiDir();
  if( !KGlobal::dirs()->exists(cepPath) ) {
    KIO::NetAccess::mkdir( cepPath, this );
  }

  // finally jump there
  chdir(cepPath.local8Bit());

  TQListViewItem *item = mPageView->selectedItem();
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item->parent() ? item->parent() : item );
    cmdLine += " " + KProcess::quote( pageItem->path() );
  }

  KRun::runCommand( cmdLine );
}

#include "kcmdesignerfields.moc"

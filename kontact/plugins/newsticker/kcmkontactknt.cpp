/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqvaluevector.h>
#include <tqspinbox.h>

#include <dcopref.h>
#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kaccelmanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klistview.h>
#include <klocale.h>
#include <kpushbutton.h>

#include "kcmkontactknt.h"

#include "newsfeeds.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactknt( TQWidget *parent, const char * )
  {
    return new KCMKontactKNT( parent, "kcmkontactknt" );
  }
}

NewsEditDialog::NewsEditDialog( const TQString& title, const TQString& url, TQWidget *parent )
  : KDialogBase( Plain, i18n( "New News Feed" ), Ok | Cancel,
                 Ok, parent, 0, true, true )
{
  TQWidget *page = plainPage();
  TQGridLayout *layout = new TQGridLayout( page, 2, 3, marginHint(),
                                         spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Name:" ), page );
  layout->addWidget( label, 0, 0 );

  mTitle = new TQLineEdit( page );
  label->setBuddy( mTitle );
  layout->addMultiCellWidget( mTitle, 0, 0, 1, 2 );

  label = new TQLabel( i18n( "URL:" ), page );
  layout->addWidget( label, 1, 0 );

  mURL = new TQLineEdit( page );
  label->setBuddy( mURL );
  layout->addMultiCellWidget( mURL, 1, 1, 1, 2 );

  mTitle->setText( title );
  mURL->setText( url );
  mTitle->setFocus();
  connect( mTitle, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( modified() ) );
  connect( mURL, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( modified() ) );

  modified();
}

void NewsEditDialog::modified()
{
  enableButton( KDialogBase::Ok, !title().isEmpty() && !url().isEmpty() );
}

TQString NewsEditDialog::title() const
{
  return mTitle->text();
}

TQString NewsEditDialog::url() const
{
  return mURL->text();
}

class NewsItem : public QListViewItem
{
  public:
    NewsItem( TQListView *parent, const TQString& title, const TQString& url, bool custom )
      : TQListViewItem( parent ), mTitle( title ), mUrl( url ), mCustom( custom )
    {
      setText( 0, mTitle );
    }

    NewsItem( TQListViewItem *parent, const TQString& title, const TQString& url, bool custom )
      : TQListViewItem( parent ), mTitle( title ), mUrl( url ), mCustom( custom )
    {
      setText( 0, mTitle );
    }

    TQString title() const { return mTitle; }
    TQString url() const { return mUrl; }
    bool custom() const { return mCustom; }

  private:
    TQString mTitle;
    TQString mUrl;
    bool mCustom;
};

KCMKontactKNT::KCMKontactKNT( TQWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  connect( mAllNews, TQT_SIGNAL( currentChanged( TQListViewItem* ) ),
           this, TQT_SLOT( allCurrentChanged( TQListViewItem* ) ) );
  connect( mSelectedNews, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
           this, TQT_SLOT( selectedChanged( TQListViewItem* ) ) );

  connect( mUpdateInterval, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( modified() ) );
  connect( mArticleCount, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( modified() ) );

  connect( mAddButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addNews() ) );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeNews() ) );
  connect( mNewButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( newFeed() ) );
  connect( mDeleteButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( deleteFeed() ) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMKontactKNT::loadNews()
{
  TQValueVector<TQListViewItem*> parents;
  TQValueVector<TQListViewItem*>::Iterator it;

  parents.append( new TQListViewItem( mAllNews, i18n( "Arts" ) ) );
  parents.append( new TQListViewItem( mAllNews, i18n( "Business" ) ) );
  parents.append( new TQListViewItem( mAllNews, i18n( "Computers" ) ) );
  parents.append( new TQListViewItem( mAllNews, i18n( "Misc" ) ) );
  parents.append( new TQListViewItem( mAllNews, i18n( "Recreation" ) ) );
  parents.append( new TQListViewItem( mAllNews, i18n( "Society" ) ) );

  for ( it = parents.begin(); it != parents.end(); ++it )
    (*it)->setSelectable( false );

  for ( int i = 0; i < DEFAULT_NEWSSOURCES; ++i ) {
    NewsSourceData data = NewsSourceDefault[ i ];
    new NewsItem( parents[ data.category() ], data.name(), data.url(), false );
    mFeedMap.insert( data.url(), data.name() );
  }
}

void KCMKontactKNT::loadCustomNews()
{
  KConfig config( "kcmkontactkntrc" );
  TQMap<TQString, TQString> customFeeds = config.entryMap( "CustomFeeds" );
  config.setGroup( "CustomFeeds" );

  mCustomItem = new TQListViewItem( mAllNews, i18n( "Custom" ) );
  mCustomItem->setSelectable( false );

  if ( customFeeds.count() == 0 )
    mCustomItem->setVisible( false );

  TQMap<TQString, TQString>::Iterator it;
  for ( it = customFeeds.begin(); it != customFeeds.end(); ++it ) {
    TQStringList value = config.readListEntry( it.key() );
    mCustomFeeds.append( new NewsItem( mCustomItem, value[ 0 ], value[ 1 ], true ) );
    mFeedMap.insert( value[ 1 ], value[ 0 ] );
    mCustomItem->setVisible( true );
  }
}

void KCMKontactKNT::storeCustomNews()
{
  KConfig config( "kcmkontactkntrc" );
  config.deleteGroup( "CustomFeeds" );
  config.setGroup( "CustomFeeds" );

  int counter = 0;
  TQValueList<NewsItem*>::Iterator it;
  for ( it = mCustomFeeds.begin(); it != mCustomFeeds.end(); ++it ) {
    TQStringList value;
    value << (*it)->title() << (*it)->url();
    config.writeEntry( TQString::number( counter ), value );

    ++counter;
  }

  config.sync();
}

void KCMKontactKNT::addNews()
{
  if ( !dcopActive() )
    return;

  NewsItem *item = dynamic_cast<NewsItem*>( mAllNews->selectedItem() );
  if ( item == 0 )
    return;

  DCOPRef service( "rssservice", "RSSService" );
  service.send( "add(TQString)", item->url() );

  scanNews();

  emit changed( true );
}

void KCMKontactKNT::removeNews()
{
  if ( !dcopActive() )
    return;

  NewsItem *item = dynamic_cast<NewsItem*>( mSelectedNews->selectedItem() );
  if ( item == 0 )
    return;

  DCOPRef service( "rssservice", "RSSService" );
  service.send( "remove(TQString)", item->url() );

  scanNews();

  emit changed( true );
}

void KCMKontactKNT::newFeed()
{
  NewsEditDialog dlg( "", "", this );

  if ( dlg.exec() ) {
    NewsItem *item = new NewsItem( mCustomItem, dlg.title(), dlg.url(), true );
    mCustomFeeds.append( item );
    mFeedMap.insert( dlg.url(), dlg.title() );

    mCustomItem->setVisible( true );
    mCustomItem->setOpen( true );
    mAllNews->ensureItemVisible( item );
    mAllNews->setSelected( item, true );

    emit changed( true );
  }
}

void KCMKontactKNT::deleteFeed()
{
  NewsItem *item = dynamic_cast<NewsItem*>( mAllNews->selectedItem() );
  if ( !item )
    return;

  if ( mCustomFeeds.find( item ) == mCustomFeeds.end() )
    return;

  mCustomFeeds.remove( item );
  mFeedMap.remove( item->url() );
  delete item;

  if ( mCustomFeeds.count() == 0 )
    mCustomItem->setVisible( false );

  emit changed( true );
}

void KCMKontactKNT::scanNews()
{
  if ( !dcopActive() )
    return;

  mSelectedNews->clear();

  DCOPRef service( "rssservice", "RSSService" );
  TQStringList urls = service.call( "list()" );

  for ( uint i = 0; i < urls.count(); ++i )
  {
    TQString url = urls[ i ];
    TQString feedName = mFeedMap[ url ];
    if ( feedName.isEmpty() )
      feedName = url;
    new NewsItem( mSelectedNews, feedName, url, false );
  }
}

void KCMKontactKNT::selectedChanged( TQListViewItem *item )
{
  mRemoveButton->setEnabled( item && item->isSelected() );
}

void KCMKontactKNT::allCurrentChanged( TQListViewItem *item )
{
  NewsItem *newsItem = dynamic_cast<NewsItem*>( item );

  bool addState = false;
  bool delState = false;
  if ( newsItem && newsItem->isSelected() ) {
    addState = true;
    delState = (mCustomFeeds.find( newsItem ) != mCustomFeeds.end());
  }

  mAddButton->setEnabled( addState );
  mDeleteButton->setEnabled( delState );
}

void KCMKontactKNT::modified()
{
  emit changed( true );
}

void KCMKontactKNT::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mAllNews = new KListView( this );
  mAllNews->addColumn( i18n( "All" ) );
  mAllNews->setRootIsDecorated( true );
  mAllNews->setFullWidth( true );
  layout->addWidget( mAllNews, 0, 0 );

  TQVBoxLayout *vbox = new TQVBoxLayout( layout, KDialog::spacingHint() );

  vbox->addStretch();
  mAddButton = new KPushButton( i18n( "Add" ), this );
  mAddButton->setEnabled( false );
  vbox->addWidget( mAddButton );
  mRemoveButton = new KPushButton( i18n( "Remove" ), this );
  mRemoveButton->setEnabled( false );
  vbox->addWidget( mRemoveButton );
  vbox->addStretch();

  mSelectedNews = new KListView( this );
  mSelectedNews->addColumn( i18n( "Selected" ) );
  mSelectedNews->setFullWidth( true );
  layout->addWidget( mSelectedNews, 0, 2 );

  TQGroupBox *box = new TQGroupBox( 0, Qt::Vertical,
                                  i18n( "News Feed Settings" ), this );

  TQGridLayout *boxLayout = new TQGridLayout( box->layout(), 2, 3,
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Refresh time:" ), box );
  boxLayout->addWidget( label, 0, 0 );

  mUpdateInterval = new TQSpinBox( 1, 3600, 1, box );
  mUpdateInterval->setSuffix( " sec." );
  label->setBuddy( mUpdateInterval );
  boxLayout->addWidget( mUpdateInterval, 0, 1 );

  label = new TQLabel( i18n( "Number of items shown:" ), box );
  boxLayout->addWidget( label, 1, 0 );

  mArticleCount = new TQSpinBox( box );
  label->setBuddy( mArticleCount );
  boxLayout->addWidget( mArticleCount, 1, 1 );

  mNewButton = new KPushButton( i18n( "New Feed..." ), box );
  boxLayout->addWidget( mNewButton, 0, 2 );

  mDeleteButton = new KPushButton( i18n( "Delete Feed" ), box );
  mDeleteButton->setEnabled( false );
  boxLayout->addWidget( mDeleteButton, 1, 2 );

  layout->addMultiCellWidget( box, 1, 1, 0, 2 );
}

bool KCMKontactKNT::dcopActive() const
{
  TQString error;
  TQCString appID;
  bool isGood = true;
  DCOPClient *client = kapp->dcopClient();
  if ( !client->isApplicationRegistered( "rssservice" ) ) {
    if ( KApplication::startServiceByDesktopName( "rssservice", TQStringList(), &error, &appID ) )
      isGood = false;
  }

  return isGood;
}

void KCMKontactKNT::load()
{
  mAllNews->clear();

  loadNews();
  loadCustomNews();
  scanNews();

  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  mUpdateInterval->setValue( config.readNumEntry( "UpdateInterval", 600 ) );
  mArticleCount->setValue( config.readNumEntry( "ArticleCount", 4 ) );

  emit changed( false );
}

void KCMKontactKNT::save()
{
  storeCustomNews();

  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  config.writeEntry( "UpdateInterval", mUpdateInterval->value() );
  config.writeEntry( "ArticleCount", mArticleCount->value() );

  config.sync();

  emit changed( false );
}

void KCMKontactKNT::defaults()
{
}

const KAboutData* KCMKontactKNT::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkontactknt" ),
                                      I18N_NOOP( "Newsticker Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkontactknt.moc"

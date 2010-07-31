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

#include <tqclipboard.h>
#include <tqeventloop.h>
#include <tqhbox.h>
#include <tqlayout.h>
#include <tqpixmap.h>
#include <tqpopupmenu.h>
#include <tqcursor.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( TQWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "NewsTickerPlugin" ), mLayout( 0 ), mFeedCounter( 0 )
{
  TQVBoxLayout *vlay = new TQVBoxLayout( this, 3, 3 );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_news",
                                                  KIcon::Desktop, KIcon::SizeMedium );

  TQWidget *header = createHeader( this, icon, i18n( "News Feeds" ) );
  vlay->addWidget( header );

  TQString error;
  TQCString appID;

  bool dcopAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "rssservice" ) ) {
    if ( KApplication::startServiceByDesktopName( "rssservice", TQStringList(), &error, &appID ) ) {
      TQLabel *label = new TQLabel( i18n( "No rss dcop service available.\nYou need rssservice to use this plugin." ), this );
      vlay->addWidget( label, Qt::AlignHCenter );
      dcopAvailable = false;
    }
  }

  mBaseWidget = new TQWidget( this, "baseWidget" );
  vlay->addWidget( mBaseWidget );

  connect( &mTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( updateDocuments() ) );

  readConfig();

  connectDCOPSignal( 0, 0, "documentUpdateError(DCOPRef,int)", "documentUpdateError(DCOPRef, int)", false );

  if ( dcopAvailable )
    initDocuments();

  connectDCOPSignal( 0, 0, "added(TQString)", "documentAdded(TQString)", false );
  connectDCOPSignal( 0, 0, "removed(TQString)", "documentRemoved(TQString)", false );
}

int SummaryWidget::summaryHeight() const
{
  return ( mFeeds.count() == 0 ? 1 : mFeeds.count() );
}

void SummaryWidget::documentAdded( TQString )
{
  initDocuments();
}

void SummaryWidget::documentRemoved( TQString )
{
  initDocuments();
}

void SummaryWidget::configChanged()
{
  readConfig();

  updateView();
}

void SummaryWidget::readConfig()
{
  KConfig config( "kcmkontactkntrc" );
  config.setGroup( "General" );

  mUpdateInterval = config.readNumEntry( "UpdateInterval", 600 );
  mArticleCount = config.readNumEntry( "ArticleCount", 4 );
}

void SummaryWidget::initDocuments()
{
  mFeeds.clear();

  DCOPRef dcopCall( "rssservice", "RSSService" );
  TQStringList urls;
  dcopCall.call( "list()" ).get( urls );

  if ( urls.isEmpty() ) { // add default
    urls.append( "http://www.kde.org/dotkdeorg.rdf" );
    dcopCall.send( "add(TQString)", urls[ 0 ] );
  }

  TQStringList::Iterator it;
  for ( it = urls.begin(); it != urls.end(); ++it ) {
    DCOPRef feedRef = dcopCall.call( "document(TQString)", *it );

    Feed feed;
    feed.ref = feedRef;
    feedRef.call( "title()" ).get( feed.title );
    feedRef.call( "link()" ).get( feed.url );
    feedRef.call( "pixmap()" ).get( feed.logo );
    mFeeds.append( feed );

    disconnectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)", 0 );
    connectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)",
                       "documentUpdated(DCOPRef)", false );

    if ( qApp )
      qApp->eventLoop()->processEvents( TQEventLoop::ExcludeUserInput |
                                    TQEventLoop::ExcludeSocketNotifiers );
  }

  updateDocuments();
}

void SummaryWidget::updateDocuments()
{
  mTimer.stop();

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
    (*it).ref.send( "refresh()" );

  mTimer.start( 1000 * mUpdateInterval );
}

void SummaryWidget::documentUpdated( DCOPRef feedRef )
{
  ArticleMap map;

  int numArticles = feedRef.call( "count()" );
  for ( int i = 0; i < numArticles; ++i ) {
    DCOPRef artRef = feedRef.call( "article(int)", i );
    TQString title, url;

    if ( qApp )
      qApp->eventLoop()->processEvents( TQEventLoop::ExcludeUserInput |
                                        TQEventLoop::ExcludeSocketNotifiers );

    artRef.call( "title()" ).get( title );
    artRef.call( "link()" ).get( url );

    QPair<TQString, KURL> article(title, KURL( url ));
    map.append( article );
  }

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
    if ( (*it).ref.obj() == feedRef.obj() ) {
      (*it).map = map;
      if ( (*it).title.isEmpty() )
        feedRef.call( "title()" ).get( (*it).title );
      if ( (*it).url.isEmpty() )
        feedRef.call( "link()" ).get( (*it).url );
      if ( (*it).logo.isNull() )
        feedRef.call( "pixmap()" ).get( (*it).logo );
    }

  mFeedCounter++;
  if ( mFeedCounter == mFeeds.count() ) {
    mFeedCounter = 0;
    updateView();
  }
}

void SummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  delete mLayout;
  mLayout = new TQVBoxLayout( mBaseWidget, 3 );

  TQFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    TQHBox *hbox = new TQHBox( mBaseWidget );
    mLayout->addWidget( hbox );

    // icon
    KURLLabel *urlLabel = new KURLLabel( hbox );
    urlLabel->setURL( (*it).url );
    urlLabel->setPixmap( (*it).logo );
    urlLabel->setMaximumSize( urlLabel->minimumSizeHint() );
    mLabels.append( urlLabel );

    connect( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
             kapp, TQT_SLOT( invokeBrowser( const TQString& ) ) );
    connect( urlLabel, TQT_SIGNAL( rightClickedURL( const TQString& ) ),
             this, TQT_SLOT( rmbMenu( const TQString& ) ) );

    // header
    TQLabel *label = new TQLabel( hbox );
    label->setText( KCharsets::resolveEntities( (*it).title ) );
    label->setAlignment( AlignLeft|AlignVCenter );
    label->setFont( boldFont );
    label->setIndent( 6 );
    label->setMaximumSize( label->minimumSizeHint() );
    mLabels.append( label );

    hbox->setMaximumWidth( hbox->minimumSizeHint().width() );
    hbox->show();

    // articles
    ArticleMap articles = (*it).map;
    ArticleMap::Iterator artIt;
    int numArticles = 0;
    for ( artIt = articles.begin(); artIt != articles.end() && numArticles < mArticleCount; ++artIt ) {
      urlLabel = new KURLLabel( (*artIt).second.url(), (*artIt).first, mBaseWidget );
      urlLabel->installEventFilter( this );
      //TODO: RichText causes too much horizontal space between articles
      //urlLabel->setTextFormat( RichText );
      mLabels.append( urlLabel );
      mLayout->addWidget( urlLabel );

      connect( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
               kapp, TQT_SLOT( invokeBrowser( const TQString& ) ) );
      connect( urlLabel, TQT_SIGNAL( rightClickedURL( const TQString& ) ),
               this, TQT_SLOT( rmbMenu( const TQString& ) ) );


      numArticles++;
    }
  }

  for ( TQLabel *label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void SummaryWidget::documentUpdateError( DCOPRef feedRef, int errorCode )
{
  kdDebug() << " error while updating document, error code: " << errorCode << endl;
  FeedList::Iterator it;
  for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
    if ( (*it).ref.obj() == feedRef.obj() ) {
      mFeeds.remove( it );
      break;
    }
  }

  if ( mFeedCounter == mFeeds.count() ) {
    mFeedCounter = 0;
    updateView();
  }

}

TQStringList SummaryWidget::configModules() const
{
  return "kcmkontactknt.desktop";
}

void SummaryWidget::updateSummary( bool )
{
  updateDocuments();
}

void SummaryWidget::rmbMenu( const TQString& url )
{
  TQPopupMenu menu;
  menu.insertItem( i18n( "Copy URL to Clipboard" ) );
  int id = menu.exec( TQCursor::pos() );
  if ( id != -1 )
    kapp->clipboard()->setText( url, QClipboard::Clipboard );
}

bool SummaryWidget::eventFilter( TQObject *obj, TQEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == TQEvent::Enter )
      emit message( label->url() );
    if ( e->type() == TQEvent::Leave )
      emit message( TQString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

#include "summarywidget.moc"

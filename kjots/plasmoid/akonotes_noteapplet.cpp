/*
    Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "akonotes_noteapplet.h"

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneResizeEvent>

#include <KJob>

#include <Plasma/FrameSvg>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/TextEdit>

#include <KMime/Message>
#include <KLineEdit>
#include <KTextEdit>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/control.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmonitor.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/monitor.h>

#include "note.h"

using namespace Akonadi;

K_EXPORT_PLASMA_APPLET( akonotes_note, AkonotesNoteApplet )

AkonotesNoteApplet::AkonotesNoteApplet( QObject* parent, const QVariantList& args )
    : Applet( parent, args ), m_monitor( new Monitor( this ) )
{
  setAspectRatioMode( Plasma::IgnoreAspectRatio );
  setBackgroundHints( Plasma::Applet::NoBackground );

  Akonadi::Control::start();

  m_subject = new Plasma::LineEdit( this );
  m_subject->installEventFilter( this );
  m_subject->setText( "Subject" );
  {
    QFont cf    = m_subject->nativeWidget()->font();
    QPalette cp = m_subject->nativeWidget()->palette();

    cf.setPointSize( cf.pointSize() - 2 );
    cp.setColor( QPalette::Active, QPalette::WindowText,
        QColor( 105, 105, 4 ) );
    cp.setColor( QPalette::Inactive, QPalette::WindowText,
        QColor( 185, 185, 84 ) );

    m_subject->nativeWidget()->setFont( cf );
    m_subject->nativeWidget()->setPalette( cp );
  }

  m_content = new Plasma::TextEdit( this );
  m_content->setText( "content" );
  m_content->installEventFilter( this );

  m_theme = new Plasma::FrameSvg( this );
  m_theme->setImagePath( "widgets/stickynote" );
  m_theme->setEnabledBorders( Plasma::FrameSvg::AllBorders );

  m_layout = new QGraphicsLinearLayout;
  m_layout->setContentsMargins( 9, 9, 9, 9 );
  m_layout->setOrientation( Qt::Vertical );
  m_layout->setSpacing( 15 );
  m_layout->addItem( m_subject );
  m_layout->addItem( m_content );
  m_layout->setStretchFactor( m_content, 220 );

  setLayout( m_layout );
  resize( 200, 200 );

  m_monitor->itemFetchScope().fetchFullPayload( true );
  connect( m_monitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), SLOT(itemChanged(Akonadi::Item)));
  connect( m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), SLOT(itemRemoved()) );

  if ( !args.isEmpty() )
  {
    m_item = Akonadi::Item::fromUrl( args.first().toString() );
  }
}

void AkonotesNoteApplet::init()
{
  KConfigGroup cg = config();

  Item::Id itemId = m_item.id();
  if ( !m_item.isValid() )
    itemId = cg.readEntry( "itemId", -1 );

  if ( itemId < 0 )
  {
    CollectionFetchJob *collectionFetchJob = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
    collectionFetchJob->fetchScope().setContentMimeTypes( QStringList() << Note::mimeType() );
    connect( collectionFetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(collectionsFetched(Akonadi::Collection::List)) );
  } else
  {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( Item( itemId ), this );
    m_monitor->setItemMonitored( Item( itemId ), true );
    job->fetchScope().fetchFullPayload( true );
    connect( job, SIGNAL(itemsReceived(Akonadi::Item::List)), SLOT(itemsFetched(Akonadi::Item::List)) );
    connect( job, SIGNAL(result(KJob *)), SLOT(itemFetchDone(KJob *)) );
  }
}

void AkonotesNoteApplet::itemFetchDone( KJob *job )
{
  if ( job->error() )
  {
    kDebug() << job->errorString();
  }
  if ( !m_item.isValid() )
  {
    CollectionFetchJob *collectionFetchJob = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
    collectionFetchJob->fetchScope().setContentMimeTypes( QStringList() << Note::mimeType() );
    connect( collectionFetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(collectionsFetched(Akonadi::Collection::List)));
  }
}


void AkonotesNoteApplet::paintInterface(QPainter* painter, const QStyleOptionGraphicsItem* option, const QRect& contentsRect)
{
  Q_UNUSED(option);
  Q_UNUSED(contentsRect);

  painter->setRenderHint(QPainter::SmoothPixmapTransform);
  painter->setRenderHint(QPainter::Antialiasing);

  painter->save();
  m_theme->paintFrame(painter);
  painter->restore();
}

void AkonotesNoteApplet::resizeEvent(QGraphicsSceneResizeEvent* event)
{
  Plasma::Applet::resizeEvent(event);
  m_theme->resizeFrame(event->newSize());
}


bool AkonotesNoteApplet::eventFilter( QObject* watched, QEvent* event )
{
  if ( ( event->type() == QEvent::FocusOut )
    && ( m_content->nativeWidget()->document()->isModified() || m_subject->nativeWidget()->isModified() )
    && ( watched == m_content || watched == m_subject ) )
  {
    saveItem();
  }
  return QObject::eventFilter(watched,event);
}

void AkonotesNoteApplet::saveItem()
{
  if ( !m_item.hasPayload<KMime::Message::Ptr>() )
    return;

  KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();

  QByteArray encoding = "utf-8";

  msg->subject()->fromUnicodeString( m_subject->text(), encoding );
  msg->mainBodyPart()->fromUnicodeString( m_content->nativeWidget()->toPlainText() );
  msg->assemble();
  m_item.setPayload( msg );

  ItemModifyJob *modifyJob = new ItemModifyJob(m_item, this);
  connect(modifyJob, SIGNAL(result(KJob*)), SLOT(modifyDone(KJob*)) );

  m_content->nativeWidget()->document()->setModified( false );
  m_subject->nativeWidget()->setModified( false );
}

void AkonotesNoteApplet::modifyDone( KJob *job )
{
  if ( job->error() )
  {
    kDebug() << job->errorString();
  }
}

void AkonotesNoteApplet::collectionsFetched( const Akonadi::Collection::List &collectionList )
{
  Collection::List possibilities;
  foreach( const Collection col, collectionList )
  {
    if ( col.id() == 325 )
//     if ( col.resource() == "defaultnotebook" )
      possibilities << col;
  }

  if ( possibilities.isEmpty() )
    return;

  Collection targetCollection;

  while ( possibilities.size() != 1 )
  {
    Collection col = possibilities.first();
    bool found = false;
    for( int i = 1; i < possibilities.size(); ++i )
    {
      if ( possibilities.at( i ).parentCollection().id() == col.id() )
      {
        found = true;
        possibilities.removeFirst();
      }
    }
    if ( !found )
      break;
  }
  targetCollection = possibilities.first();

  Item item;
  item.setMimeType( Note::mimeType() );

  KMime::Message::Ptr msg = KMime::Message::Ptr( new KMime::Message() );

  QString title = i18nc( "The default name for new pages.", "New Page" );
  QByteArray encoding( "utf-8" );

  msg->subject( true )->fromUnicodeString( title, encoding );
  msg->contentType( true )->setMimeType( "text/plain" );
  msg->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
  // Need a non-empty body part so that the serializer regards this as a valid message.
  msg->mainBodyPart()->fromUnicodeString( " " );

  msg->assemble();

  item.setPayload( msg );

  ItemCreateJob *job = new ItemCreateJob(item, targetCollection);
  connect( job, SIGNAL(result(KJob*)), SLOT(itemCreateJobFinished( KJob *)));
}

void AkonotesNoteApplet::itemsFetched( const Akonadi::Item::List& itemList )
{
  Q_ASSERT( itemList.size() == 1 );
  Akonadi::Item item = itemList.first();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
  {
    CollectionFetchJob *collectionFetchJob = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
    collectionFetchJob->fetchScope().setContentMimeTypes( QStringList() << Note::mimeType() );
    connect( collectionFetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(collectionsFetched(Akonadi::Collection::List)));
    return;
  }

  KConfigGroup cg = config();
  cg.writeEntry( "itemId", item.id() );

  itemChanged( item );
}

void AkonotesNoteApplet::itemCreateJobFinished( KJob *job )
{
  if ( job->error() )
  {
    kDebug() << job->errorString();
    return;
  }

  Akonadi::ItemCreateJob *createJob = qobject_cast<Akonadi::ItemCreateJob *>( job );

  if (!createJob)
    return;

  Akonadi::Item item = createJob->item();

  m_monitor->setItemMonitored( item, true );

  KConfigGroup cg = config();
  cg.writeEntry( "itemId", item.id() );

  itemChanged( item );
}

void AkonotesNoteApplet::itemChanged( const Akonadi::Item& item )
{
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  KMime::Headers::Subject *subject = msg->subject();
  m_subject->setText( subject->asUnicodeString() );
  m_content->nativeWidget()->setText( msg->mainBodyPart()->decodedText() );
  m_item = item;
}

void AkonotesNoteApplet::itemRemoved()
{
  destroy();
}

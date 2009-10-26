/*
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

#include "akonotes_applet.h"

#include <plasma/svg.h>
#include <plasma/widgets/textedit.h>
#include <plasma/extenderitem.h>

#include <QtGui/QPainter>

#include <kdescendantsproxymodel.h>

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/changerecorder.h>
#include <akonadi/session.h>

#include <KMime/KMimeMessage>

using namespace Akonadi;

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET( akonotes, AkonotesMasterApplet )

AkonotesMasterApplet::AkonotesMasterApplet( QObject *parent, const QVariantList &args )
    : Plasma::PopupApplet( parent, args ), m_svg(this)//, w( 0 )
{
  setPopupIcon( QLatin1String( "kjots" ) );

  m_svg.setImagePath( QLatin1String( "/home/kde-devel/kde/src/akonadi-ports/kdepim/kjots/plasmoid/background.svg" ) );
  // this will get us the standard applet background, for free!
//   setBackgroundHints(DefaultBackground);
  resize(200, 200);

  ItemFetchScope scope;
  scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
  scope.fetchAttribute< EntityDisplayAttribute >();

  ChangeRecorder *monitor = new ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );
  monitor->setCollectionMonitored( Collection::root() );
  monitor->setMimeTypeMonitored( QLatin1String( "text/x-vnd.akonadi.note" ) );

  Session *session = new Session( QByteArray( "EntityTreeModel-" ) + QByteArray::number( qrand() ), this );

  EntityTreeModel *model = new EntityTreeModel( session, monitor, this );

  KDescendantsProxyModel *descsProxy = new KDescendantsProxyModel(this);
  descsProxy->setSourceModel( model );

  EntityMimeTypeFilterModel *filter = new EntityMimeTypeFilterModel(this);
  filter->addMimeTypeExclusionFilter( Collection::mimeType() );
  filter->setSourceModel( descsProxy );

  m_model = filter;

  connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(itemsAdded(QModelIndex,int,int)) );

}


AkonotesMasterApplet::~AkonotesMasterApplet()
{
  if ( hasFailedToLaunch() )
  {
    // Nothing to do yet
  }
}

void AkonotesMasterApplet::init()
{
}

void AkonotesMasterApplet::initExtenderItem( Plasma::ExtenderItem *item, const QModelIndex &idx )
{
    Plasma::TextEdit *textEdit = new Plasma::TextEdit( item );

    Item akonadiItem = idx.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    Q_ASSERT( akonadiItem.isValid() );

    if (!akonadiItem.hasPayload<KMime::Message::Ptr>())
      return;

    KMime::Message::Ptr msg = akonadiItem.payload<KMime::Message::Ptr>();

    textEdit->setText( msg->mainBodyPart()->decodedText() );

    item->setTitle( msg->subject()->asUnicodeString() );

    item->setWidget( textEdit );

}

void AkonotesMasterApplet::paintInterface(QPainter *p,
        const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    p->setRenderHint( QPainter::SmoothPixmapTransform );
    p->setRenderHint( QPainter::Antialiasing );

    // Now we draw the applet, starting with our svg
    m_svg.resize( static_cast<int>( contentsRect.width() ), static_cast<int>( contentsRect.height() ) );
    m_svg.paint( p, static_cast<int>( contentsRect.left() ), static_cast<int>( contentsRect.top() ) );
}

void AkonotesMasterApplet::itemsAdded(const QModelIndex &parent, int start, int end )
{
  const int column = 0;
  for(int row = start; row <= end; ++row)
  {
    if ((row-start) > 1)
      return;
    kDebug() << "creating";
    Plasma::ExtenderItem *item = new Plasma::ExtenderItem(extender());
    QModelIndex idx = m_model->index(row, column, parent);
    Q_ASSERT( idx.isValid() );
    initExtenderItem(item, idx);
    // TODO: Give it a name
  }
}

#include "akonotes_applet.moc"

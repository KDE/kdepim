/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "item.h"
#include "core/item.h"
#include "core/item_p.h"
#include "core/model.h"
#include "core/manager.h"

#include <kio/global.h> // for KIO::filesize_t and related functions
#include <kmime/kmime_dateformatter.h> // kdepimlibs

#include <KLocale>

using namespace MessageList::Core;

Item::Item( Type type )
  : d( new ItemPrivate( this ) )
{
  d->mType = type;
  d->mChildItems = 0;
  d->mParent = 0;
  d->mThisItemIndexGuess = 0;
  d->mIsViewable = false;
  d->mInitialExpandStatus = NoExpandNeeded;
}

Item::~Item()
{
  killAllChildItems();

  if ( d->mParent )
    d->mParent->d->childItemDead( this );

  delete d;
}

void Item::childItemStats( ChildItemStats &stats ) const
{
  Q_ASSERT( d->mChildItems );

  stats.mTotalChildCount += d->mChildItems->count();
  for( QList< Item * >::Iterator it = d->mChildItems->begin(); it != d->mChildItems->end(); ++it )
  {
    if ( ( *it )->status().isUnread() )
      stats.mUnreadChildCount++;
    if ( ( *it )->d->mChildItems )
      ( *it )->childItemStats( stats );
  }
}

QList< Item * > *Item::childItems() const
{
  return d->mChildItems;
}

Item *Item::childItem( int idx ) const
{
  if ( idx < 0 )
    return 0;
  if ( !d->mChildItems )
    return 0;
  if ( d->mChildItems->count() <= idx )
    return 0;
  return d->mChildItems->at( idx );
}

Item * Item::firstChildItem() const
{
  return d->mChildItems ? ( d->mChildItems->count() > 0 ? d->mChildItems->at( 0 ) : 0 ) : 0;
}

Item * Item::itemBelowChild( Item * child )
{
  Q_ASSERT( d->mChildItems );

  int idx = child->indexGuess();
  if ( !childItemHasIndex( child, idx ) )
  {
    idx = d->mChildItems->indexOf( child );
    child->setIndexGuess( idx );
  }
  Q_ASSERT( idx >= 0 );

  idx++;

  if ( idx < d->mChildItems->count() )
    return d->mChildItems->at( idx );

  if ( !d->mParent )
    return 0;

  return d->mParent->itemBelowChild( this );
}

Item * Item::itemBelow()
{
  if ( d->mChildItems )
  {
    if ( d->mChildItems->count() > 0 )
      return d->mChildItems->at( 0 );
  }

  if ( !d->mParent )
    return 0;

  return d->mParent->itemBelowChild( this );
}

Item * Item::deepestItem()
{
  if ( d->mChildItems )
  {
    if ( d->mChildItems->count() > 0 )
      return d->mChildItems->at( d->mChildItems->count() - 1 )->deepestItem();
  }

  return this;
}

Item * Item::itemAboveChild( Item * child )
{
  if ( d->mChildItems )
  {
    int idx = child->indexGuess();
    if ( !childItemHasIndex( child, idx ) )
    {
      idx = d->mChildItems->indexOf( child );
      child->setIndexGuess( idx );
    }
    Q_ASSERT( idx >= 0 );
    idx--;

    if ( idx >= 0 )
      return d->mChildItems->at( idx );
  }

  return this;
}

Item * Item::itemAbove()
{
  if ( !d->mParent )
    return 0;

  Item *siblingAbove = d->mParent->itemAboveChild( this );
  if ( siblingAbove && siblingAbove != this && siblingAbove != d->mParent &&
       siblingAbove->childItemCount() > 0 )
  {
    return siblingAbove->deepestItem();
  }

  return d->mParent->itemAboveChild( this );
}

int Item::childItemCount() const
{
  return d->mChildItems ? d->mChildItems->count() : 0;
}

bool Item::hasChildren() const
{
  return childItemCount() > 0;
}

int Item::indexOfChildItem( Item *item ) const
{
  return d->mChildItems ? d->mChildItems->indexOf( item ) : -1;
}

int Item::indexGuess() const
{
  return d->mThisItemIndexGuess;
}

void Item::setIndexGuess( int index )
{
  d->mThisItemIndexGuess = index;
}

bool Item::childItemHasIndex( const Item *item, int idx ) const
{
  return d->mChildItems ? ( ( d->mChildItems->count() > idx ) ? ( d->mChildItems->at( idx ) == item ) : false ) : false;
}

Item * Item::topmostNonRoot()
{
  Q_ASSERT( d->mType != InvisibleRoot );

  if ( !d->mParent )
    return this;

  if ( d->mParent->type() == InvisibleRoot )
    return this;

  return d->mParent->topmostNonRoot();
}


static inline void append_string( QString &buffer, const QString &append )
{
  if ( !buffer.isEmpty() )
    buffer += ", ";
  buffer += append;
}

QString Item::statusDescription() const
{
  QString ret;
  if( status().isUnread() )
    append_string( ret, i18nc( "Status of an item", "Unread" ) );
  else
    append_string( ret, i18nc( "Status of an item", "Read" ) );

  if( status().hasAttachment() )
    append_string( ret, i18nc( "Status of an item", "Has Attachment" ) );

  if( status().isReplied() )
    append_string( ret, i18nc( "Status of an item", "Replied" ) );

  if( status().isForwarded() )
    append_string( ret, i18nc( "Status of an item", "Forwarded" ) );

  if( status().isSent() )
    append_string( ret, i18nc( "Status of an item", "Sent" ) );

  if( status().isImportant() )
    append_string( ret, i18nc( "Status of an item", "Important" ) );

  if( status().isToAct() )
    append_string( ret, i18nc( "Status of an item", "Action Item" ) );

  if( status().isSpam() )
    append_string( ret, i18nc( "Status of an item", "Spam" ) );

  if( status().isHam() )
    append_string( ret, i18nc( "Status of an item", "Ham" ) );

  if( status().isWatched() )
    append_string( ret, i18nc( "Status of an item", "Watched" ) );

  if( status().isIgnored() )
    append_string( ret, i18nc( "Status of an item", "Ignored" ) );

  return ret;
}

const QString & Item::formattedSize()
{
  if ( d->mFormattedSize.isEmpty() )
    d->mFormattedSize = KIO::convertSize( ( KIO::filesize_t ) size() );
  return d->mFormattedSize;
}

const QString & Item::formattedDate()
{
  if ( d->mFormattedDate.isEmpty() )
  {
    if ( static_cast< uint >( date() ) == static_cast< uint >( -1 ) )
      d->mFormattedDate = Manager::instance()->cachedLocalizedUnknownText();
    else
      d->mFormattedDate = Manager::instance()->dateFormatter()->dateString( date() );
  }
  return d->mFormattedDate;
}

const QString & Item::formattedMaxDate()
{
  if ( d->mFormattedMaxDate.isEmpty() )
  {
    if ( static_cast< uint >( maxDate() ) == static_cast< uint >( -1 ) )
      d->mFormattedMaxDate = Manager::instance()->cachedLocalizedUnknownText();
    else
      d->mFormattedMaxDate = Manager::instance()->dateFormatter()->dateString( maxDate() );
  }
  return d->mFormattedMaxDate;
}

bool Item::recomputeMaxDate()
{
  time_t newMaxDate = d->mDate;

  if ( d->mChildItems )
  {
    for ( QList< Item * >::ConstIterator it = d->mChildItems->constBegin(); it != d->mChildItems->constEnd(); ++it )
    {
      if ( ( *it )->d->mMaxDate > newMaxDate )
        newMaxDate = ( *it )->d->mMaxDate;
    }
  }

  if ( newMaxDate != d->mMaxDate )
  {
    setMaxDate( newMaxDate );
    return true;
  }
  return false;
}


Item::Type Item::type() const
{
  return d->mType;
}

Item::InitialExpandStatus Item::initialExpandStatus() const
{
  return d->mInitialExpandStatus;
}

void Item::setInitialExpandStatus( InitialExpandStatus initialExpandStatus )
{
  d->mInitialExpandStatus = initialExpandStatus;
}

bool Item::isViewable() const
{
  return d->mIsViewable;
}

bool Item::hasAncestor( const Item * it ) const
{
  return d->mParent ? ( d->mParent == it ? true : d->mParent->hasAncestor( it ) ) : false;
}

void Item::setViewable( Model *model,bool bViewable )
{
  if ( d->mIsViewable == bViewable )
    return;

  if ( !d->mChildItems )
  {
    d->mIsViewable = bViewable;
    return;
  }

  if ( d->mChildItems->count() < 1 )
  {
    d->mIsViewable = bViewable;
    return;
  }

  if ( bViewable )
  {
    if ( model )
    {
      // fake having no children, for a second
      QList< Item * > * tmp = d->mChildItems;
      d->mChildItems = 0;
      //qDebug("BEGIN INSERT ROWS FOR PARENT %x: from %d to %d, (will) have %d children",this,0,tmp->count()-1,tmp->count());
      model->beginInsertRows( model->index( this, 0 ), 0, tmp->count() - 1 );
      d->mChildItems = tmp;
      d->mIsViewable = true;
      model->endInsertRows();
    } else {
      d->mIsViewable = true;
    }

    for ( QList< Item * >::Iterator it = d->mChildItems->begin(); it != d->mChildItems->end() ;++it )
     ( *it )->setViewable( model, bViewable );
  } else {
    for ( QList< Item * >::Iterator it = d->mChildItems->begin(); it != d->mChildItems->end() ;++it )
      ( *it )->setViewable( model, bViewable );

    // It seems that we can avoid removing child items here since the parent has been removed: this is a hack tough
    // and should check if Qt4 still supports it in the next (hopefully largely fixed) release

    if ( model )
    {
      // fake having no children, for a second
      model->beginRemoveRows( model->index( this, 0 ), 0, d->mChildItems->count() - 1 );
      QList< Item * > * tmp = d->mChildItems;
      d->mChildItems = 0;
      d->mIsViewable = false;
      model->endRemoveRows();
      d->mChildItems = tmp;
    } else {
      d->mIsViewable = false;
    }
  }
}

void Item::killAllChildItems()
{
  if ( !d->mChildItems )
    return;

  while( !d->mChildItems->isEmpty() )
    delete d->mChildItems->first(); // this will call childDead() which will remove the child from the list

  delete d->mChildItems;
  d->mChildItems = 0;
}

Item * Item::parent() const
{
  return d->mParent;
}

void Item::setParent( Item *pParent )
{
  d->mParent = pParent;
}

const KPIM::MessageStatus &Item::status() const
{
  return d->mStatus;
}

void Item::setStatus( const KPIM::MessageStatus &status )
{
  d->mStatus = status;
}

size_t Item::size() const
{
  return d->mSize;
}

void Item::setSize( size_t size )
{
  d->mSize = size;
  d->mFormattedSize.clear();
}

time_t Item::date() const
{
  return d->mDate;
}

void Item::setDate( time_t date )
{
  d->mDate = date;
  d->mFormattedDate.clear();
}

time_t Item::maxDate() const
{
  return d->mMaxDate;
}

void Item::setMaxDate( time_t date )
{
  d->mMaxDate = date;
  d->mFormattedMaxDate.clear();
}

const QString &Item::sender() const
{
  return d->mSender;
}

void Item::setSender( const QString &sender )
{
  d->mSender = sender;
}

const QString &Item::receiver() const
{
  return d->mReceiver;
}

void Item::setReceiver( const QString &receiver )
{
  d->mReceiver = receiver;
}

const QString &Item::senderOrReceiver() const
{
  return d->mSenderOrReceiver;
}

void Item::setSenderOrReceiver( const QString &senderOrReceiver )
{
  d->mSenderOrReceiver = senderOrReceiver;
}

const QString &Item::subject() const
{
  return d->mSubject;
}

void Item::setSubject( const QString &subject )
{
  d->mSubject = subject;
}

void MessageList::Core::Item::initialSetup( time_t date, size_t size,
                                            const QString &sender,
                                            const QString &receiver,
                                            const QString &senderOrReceiver )
{
  d->mDate = date;
  d->mMaxDate = date;
  d->mSize = size;
  d->mSender = sender;
  d->mReceiver = receiver;
  d->mSenderOrReceiver = senderOrReceiver;
}

void MessageList::Core::Item::setSubjectAndStatus(const QString &subject,
                                                  const KPIM::MessageStatus &status)
{
  d->mSubject = subject;
  d->mStatus = status;
}

// FIXME: Try to "cache item insertions" and call beginInsertRows() and endInsertRows() in a chunked fashion...

void Item::rawAppendChildItem( Item * child )
{
  if ( !d->mChildItems )
    d->mChildItems = new QList< Item * >();
  d->mChildItems->append( child );
}

int Item::appendChildItem( Model *model, Item *child )
{
  if ( !d->mChildItems )
    d->mChildItems = new QList< Item * >();
  int idx = d->mChildItems->count();
  if ( d->mIsViewable )
  {
    if ( model )
      model->beginInsertRows( model->index( this, 0 ), idx, idx ); // THIS IS EXTREMELY UGLY, BUT IT'S THE ONLY POSSIBLE WAY WITH QT4 AT THE TIME OF WRITING
    d->mChildItems->append( child );
    child->setIndexGuess( idx );
    if ( model )
      model->endInsertRows(); // THIS IS EXTREMELY UGLY, BUT IT'S THE ONLY POSSIBLE WAY WITH QT4 AT THE TIME OF WRITING
    child->setViewable( model, true );
  } else {
    d->mChildItems->append( child );
    child->setIndexGuess( idx );
  }
  return idx;
}


void Item::dump( const QString &prefix )
{
  QString out = QString("%1 %x VIEWABLE:%2").arg(prefix).arg(d->mIsViewable ? "yes" : "no");
  qDebug( out.toUtf8().data(),this );

  QString nPrefix = prefix;
  nPrefix += QString("  ");

  if (!d->mChildItems )
    return;

  for ( QList< Item * >::Iterator it = d->mChildItems->begin(); it != d->mChildItems->end() ;++it )
    (*it)->dump(nPrefix);
}

void Item::takeChildItem( Model *model, Item *child )
{
  if ( !d->mChildItems )
    return; // Ugh... not our child ?

  if ( !d->mIsViewable )
  {
    //qDebug("TAKING NON VIEWABLE CHILD ITEM %x",child);
    // We can highly optimize this case
    d->mChildItems->removeOne( child );
#if 0
    // This *could* be done, but we optimize and avoid it.
    if ( d->mChildItems->count() < 1 )
    {
      delete d->mChildItems;
      d->mChildItems = 0;
    }
#endif
    child->setParent( 0 );
    return;
  }

  // Can't optimize: must call the model functions
  int idx = child->indexGuess();
  if ( d->mChildItems->count() > idx )
  {
    if ( d->mChildItems->at( idx ) != child ) // bad guess :/
      idx = d->mChildItems->indexOf( child );
  } else
    idx = d->mChildItems->indexOf( child ); // bad guess :/

  if ( idx < 0 )
    return; // Aaargh... not our child ?

  child->setViewable( model, false );
  if ( model )
    model->beginRemoveRows( model->index( this, 0 ), idx, idx );
  child->setParent( 0 );
  d->mChildItems->removeAt( idx );
  if ( model )
    model->endRemoveRows();

#if 0
  // This *could* be done, but we optimize and avoid it.
  if ( d->mChildItems->count() < 1 )
  {
    delete d->mChildItems;
    d->mChildItems = 0;
  }
#endif
}


void ItemPrivate::childItemDead( Item *child )
{
  // mChildItems MUST be non zero here, if it's not then it's a bug in THIS FILE
  mChildItems->removeOne( child ); // since we always have ONE (if we not, it's a bug)
}

#include <klocale.h>

#include "konnectorpairmanager.h"

#include "konnectorpairview.h"

KonnectorPairItem::KonnectorPairItem( KonnectorPair *pair, KListView *parent )
  : QObject( 0 ), QListViewItem( parent ), mPair( pair )
{
  connect( pair->manager(), SIGNAL( synceesRead( Konnector* ) ),
           this, SLOT( synceesRead( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeReadError( Konnector* ) ),
           this, SLOT( synceeReadError( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceesWritten( Konnector* ) ),
           this, SLOT( synceesWritten( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeWriteError( Konnector* ) ),
           this, SLOT( synceeWriteError( Konnector* ) ) );

  mStatusMsg = i18n( "Press \"Sync\" to synchronize" );
}

QString KonnectorPairItem::text( int column ) const
{
  switch ( column ) {
    case 0:
      return i18n( "Yes" );
      break;
    case 1:
      return mPair->name();
      break;
    case 2:
      return mStatusMsg;
      break;
    default:
      return QString::null;
  }
}

QString KonnectorPairItem::uid() const
{
  return mPair->uid();
}

void KonnectorPairItem::synceesRead( Konnector *konnector )
{
  mStatusMsg = i18n( "Retrieve data from %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceeReadError( Konnector *konnector )
{
  mStatusMsg = i18n( "Couldn't retrieve data from %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceesWritten( Konnector *konnector )
{
  mStatusMsg = i18n( "Write back data to %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceeWriteError( Konnector *konnector )
{
  mStatusMsg = i18n( "Couldn't write back data to %1..." ).arg( konnector->resourceName() );
  repaint();
}



KonnectorPairView::KonnectorPairView( KonnectorPairManager* manager, QWidget *parent )
  : KListView( parent ), mManager( manager )
{
  addColumn( i18n( "Enabled" ) );
  addColumn( i18n( "Name" ) );
  addColumn( i18n( "State" ) );

  setAllColumnsShowFocus( true );
  setFullWidth( true );

  connect( manager, SIGNAL( changed() ), this, SLOT( refreshView() ) );
}

KonnectorPairView::~KonnectorPairView()
{
}

QString KonnectorPairView::selectedPair() const
{
  KonnectorPairItem *item = static_cast<KonnectorPairItem*>( selectedItem() );
  if ( item )
    return item->uid();
  else
    return QString::null;
}

void KonnectorPairView::refresh()
{
  refreshView();
}

void KonnectorPairView::refreshView()
{
  clear();

  KonnectorPair::List pairs = mManager->pairs();
  KonnectorPair::List::Iterator it;
  for ( it = pairs.begin(); it != pairs.end(); ++it )
    new KonnectorPairItem( *it, this );

  setSelected( firstChild(), true );
}

#include "konnectorpairview.moc"

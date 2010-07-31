#ifndef __FOLDERVIEWTOOLTIP_H__
#define __FOLDERVIEWTOOLTIP_H__

#include <kmfoldercachedimap.h>

#include <tqtooltip.h>

namespace KMail {

class FolderViewToolTip : public QToolTip
{
  public:
    FolderViewToolTip( TQListView* parent ) :
      TQToolTip( parent->viewport() ),
      mListView( parent ) {}

  protected:
    void maybeTip( const TQPoint &point )
    {
      KMFolderTreeItem *item = dynamic_cast<KMFolderTreeItem*>( mListView->itemAt( point ) );
      if  ( !item )
        return;
      const TQRect itemRect = mListView->itemRect( item );
      if ( !itemRect.isValid() )
        return;
      const TQRect headerRect = mListView->header()->sectionRect( 0 );
      if ( !headerRect.isValid() )
        return;
      
      if ( !item->folder() || item->folder()->noContent() )
        return;
      
      item->updateCount();
      TQString tipText = i18n("<qt><b>%1</b><br>Total: %2<br>Unread: %3<br>Size: %4" )
          .arg( item->folder()->prettyURL().replace( " ", "&nbsp;" ) )
          .arg( item->totalCount() < 0 ? "-" : TQString::number( item->totalCount() ) )
          .arg( item->unreadCount() < 0 ? "-" : TQString::number( item->unreadCount() ) )
          .arg( KIO::convertSize( item->folderSize() ) );
      
      if ( KMFolderCachedImap* imap = dynamic_cast<KMFolderCachedImap*>( item->folder()->storage() ) ) {
          QuotaInfo info = imap->quotaInfo();
          if ( info.isValid() && !info.isEmpty() )
              tipText += i18n("<br>Quota: %1").arg( info.toString() );
      }
      
      tip( TQRect( headerRect.left(), itemRect.top(), headerRect.width(), itemRect.height() ), tipText );
    }

  private:
    TQListView *mListView;
};

}

#endif /* __FOLDERVIEWTOOLTIP_H__ */

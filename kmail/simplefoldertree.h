/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>
    Copyright (c) Stefan Taferner <taferner@kde.org>

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
*/

#ifndef KMAIL_SIMPLEFOLDERTREE_H
#define KMAIL_SIMPLEFOLDERTREE_H

#include "kmfolder.h"
#include "kmfoldertree.h"
#include "treebase.h"

#include <kdebug.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

class KMFolder;
class KMFolderTree;

namespace KMail {

static int recurseFilter( TQListViewItem * item, const TQString& filter, int column )
{
    if ( item == 0 )
      return 0;

    TQListViewItem * child;
    child = item->firstChild();

    int enabled = 0;
    while ( child ) {
      enabled += recurseFilter( child, filter, column );
      child = child->nextSibling();
    }

    if ( filter.length() == 0 ||
         item->text( column ).find( filter, 0, false ) >= 0 ) {
      item->setVisible( true );
      ++enabled;
    }
    else {
          item->setVisible( !!enabled );
           item->setEnabled( false );
    }

         return enabled;
}

class TreeItemBase 
{
   public :
   TreeItemBase()
     : mFolder( 0 )
   {
     kdDebug(5006) << k_funcinfo << endl;
   }
   virtual ~TreeItemBase() { }

   void setFolder( KMFolder * folder ) { mFolder = folder; };
   const KMFolder * folder() { return mFolder; };

    // Set the flag which determines if this is an alternate row
   void setAlternate ( bool alternate ) {
	mAlternate = alternate;
   }

   private:
     KMFolder * mFolder;
     bool mAlternate;
			
};

template <class T> class SimpleFolderTreeItem : public T, public TreeItemBase
{
  public:
    SimpleFolderTreeItem( TQListView * listView ) : 
        T( listView ), TreeItemBase() 
    {
     kdDebug(5006) << k_funcinfo << endl;
    }
    SimpleFolderTreeItem( TQListView * listView, TQListViewItem * afterListViewItem ) :
            T( listView, afterListViewItem ) , TreeItemBase() 
    {
     kdDebug(5006) << k_funcinfo << endl;
    }
    SimpleFolderTreeItem( TQListViewItem * listViewItem ) : T( listViewItem ) , TreeItemBase() 
    {
     kdDebug(5006) << k_funcinfo << endl;
    }

    SimpleFolderTreeItem( TQListViewItem * listViewItem, TQListViewItem * afterListViewItem ) :
            T( listViewItem, afterListViewItem ) , TreeItemBase() 
    {
     kdDebug(5006) << k_funcinfo << endl;
    }

};

template <> class SimpleFolderTreeItem<TQCheckListItem> : public TQCheckListItem, public TreeItemBase
{
  public:
    SimpleFolderTreeItem( TQListView * listView ) :
        TQCheckListItem( listView, TQString(), CheckBox ), TreeItemBase()  {}
    SimpleFolderTreeItem( TQListView * listView, TQListViewItem * afterListViewItem ) :
            TQCheckListItem( listView, afterListViewItem, TQString(), CheckBox ), TreeItemBase()  {}
    SimpleFolderTreeItem( TQListViewItem * listViewItem ) :
            TQCheckListItem( listViewItem, TQString(), CheckBox ) {}
    SimpleFolderTreeItem( TQListViewItem * listViewItem, TQListViewItem * afterListViewItem ) :
            TQCheckListItem( listViewItem, afterListViewItem, TQString(), CheckBox ) {}

};


template <class T> class SimpleFolderTreeBase : public TreeBase
{
  
   public:


    inline SimpleFolderTreeBase( TQWidget * parent, KMFolderTree *folderTree,
                      const TQString &preSelection, bool mustBeReadWrite )
        : TreeBase( parent, folderTree, preSelection, mustBeReadWrite )
    {
      assert( folderTree );
      setFolderColumn( addColumn( i18n( "Folder" ) ) );
      setPathColumn( addColumn( i18n( "Path" ) ) );
      
      setRootIsDecorated( true );
      setSorting( -1 );

      reload( mustBeReadWrite, true, true, preSelection );

    }

    virtual SimpleFolderTreeItem<T>* createItem( TQListView * parent )
    {
        return new SimpleFolderTreeItem<T>( parent );
    }

    virtual SimpleFolderTreeItem<T>* createItem( TQListView * parent, TQListViewItem* afterListViewItem  )
    {
        return new SimpleFolderTreeItem<T>( parent, afterListViewItem );
    }

    virtual SimpleFolderTreeItem<T>* createItem( TQListViewItem * parent, TQListViewItem* afterListViewItem )
    {
        return new SimpleFolderTreeItem<T>( parent, afterListViewItem );
    }

    virtual SimpleFolderTreeItem<T>* createItem( TQListViewItem * parent )
    {
        return new SimpleFolderTreeItem<T>( parent );
    }

    inline void keyPressEvent( TQKeyEvent *e )
    {
      const char ascii = e->ascii();
      if ( ascii == 8 || ascii == 127 ) {
        if ( mFilter.length() > 0 ) {
          mFilter.truncate( mFilter.length()-1 );
          applyFilter( mFilter );
        }
      } else if ( !e->text().isEmpty() && e->text().length() == 1 && e->text().at( 0 ).isPrint() ) {
        applyFilter( mFilter + e->text() );
      } else {
        KListView::keyPressEvent( e );
      }
    }

    void applyFilter( const TQString& filter )
    {
      kdDebug(5006) << k_funcinfo << filter << endl ;
      // Reset all items to visible, enabled, and open
      TQListViewItemIterator clean( this );
      while ( clean.current() ) {
        TQListViewItem * item = clean.current();
        item->setEnabled( true );
        item->setVisible( true );
        item->setOpen( true );
        ++clean;
      }

      mFilter = filter;

      if ( filter.isEmpty() ) {
        setColumnText( pathColumn(), i18n("Path") );
        return;
      }

      // Set the visibility and enabled status of each list item.
      // The recursive algorithm is necessary because visiblity
      // changes are automatically applied to child nodes by TQt.
      TQListViewItemIterator it( this );
      while ( it.current() ) {
        TQListViewItem * item = it.current();
        if ( item->depth() <= 0 )
          recurseFilter( item, filter, pathColumn() );
        ++it;
      }

      // Recolor the rows appropriately
      recolorRows();

      // Iterate through the list to find the first selectable item
      TQListViewItemIterator first ( this );
      while ( first.current() ) {
        SimpleFolderTreeItem<T> * item = static_cast< SimpleFolderTreeItem<T> * >( first.current() );

        if ( item->isVisible() && item->isSelectable() ) {
          setSelected( item, true );
          ensureItemVisible( item );
          break;
        }

        ++first;
      }

      // Display and save the current filter
      if ( filter.length() > 0 )
        setColumnText( pathColumn(), i18n("Path") + "  ( " + filter + " )" );
      else
        setColumnText( pathColumn(), i18n("Path") );

      mFilter = filter;
    }

};

typedef SimpleFolderTreeBase<KListViewItem> SimpleFolderTree;

}

#endif

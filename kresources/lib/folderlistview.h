/* This file is part of the KDE libraries

   Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
   
   Taken in large parts from the kate highlighting list view kateschema.h:
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef FOLDERLISTVIEW_H
#define FOLDERLISTVIEW_H

#include <klistview.h>
#include "folderlister.h"

class FolderListItem;
class FolderListCaption;


/*
    QListView that automatically adds columns for FolderListItems for selecting
    the default destination and a slot to edit the destinations using the keyboard.
*/
class FolderListView : public KListView
{
  Q_OBJECT

  friend class FolderListItem;

  public:
    /* mainly for readability */
    enum Property { FolderName, Event, Todo, Journal, Contact, All, Unknown, PROP_MAX };
    
    FolderListView( QWidget *parent, const QValueList<Property> &types = QValueList<Property>() );
    ~FolderListView() {};

    /* Display a popupmenu for item i at the specified global position, eventually with a title,
       promoting the context name of that item */
    void showPopupMenu( FolderListItem *i, const QPoint &globalPos );
    void emitChanged() { emit changed(); };
    void setEnabledTypes( const QValueList<Property> &types );

    int columnForType( Property prop ) const { if ( mColumnMap.contains(prop) ) return mColumnMap[prop]; else return -1;}
    Property typeForColumn( int col ) const { if ( mTypeMap.contains( col ) ) return mTypeMap[col]; else return Unknown; }

  private slots:
    /* Display a popupmenu for item i at item position */
    void showPopupMenu( QListViewItem *i );
    /* call item to change a property, or display a menu */
    void slotMousePressed( int, QListViewItem*, const QPoint&, int );
    /* asks item to change the property in q */
    void slotPopupHandler( int z );

  signals:
    void changed();
  private:
    QValueList<Property> mTypes;
    QMap<Property,int> mColumnMap;
    QMap<int,Property> mTypeMap;
};

/*
    QListViewItem subclass to display/edit a folder on a groupware server. 
    Selection of default destinations will be done via radio items.
*/
class FolderListItem : public QCheckListItem
{
  typedef QCheckListItem super;
  public:
    FolderListItem( FolderListItem *parent, const KPIM::FolderLister::Entry &folder )
      : QCheckListItem( parent, folder.name, QCheckListItem::CheckBoxController ), 
        mFolder( folder ), mFolderListView( parent?(parent->folderListView()):0 )
    {
      setOn( mFolder.active );
    }
    FolderListItem( FolderListView *listView, const KPIM::FolderLister::Entry &folder )
      : QCheckListItem( listView, folder.name,
          QCheckListItem::CheckBoxController ), mFolder( folder ), mFolderListView( listView )
    {
      setOn( mFolder.active );
    }

    KPIM::FolderLister::Entry folder() const
    {
      return mFolder;
    }

    /* reimp */
//     int width ( const QFontMetrics & fm, const QListView * lv, int c ) const;

    bool typeSupported( FolderListView::Property prop );
    bool isDefault( FolderListView::Property prop );
    void setDefault( FolderListView::Property prop, bool def = true );

    /* calls changeProperty() if it makes sense considering pos. */
    void activate( int column, const QPoint &localPos );
    /* Sets this item as default for property p a */
    void changeProperty( FolderListView::Property p );

    FolderListView *folderListView() const { return mFolderListView; }
    
  protected:
    /* reimp */
    void paintCell(QPainter *p, const QColorGroup& cg, int col, int width, int align);    
              
  private:
    KPIM::FolderLister::Entry mFolder;
    bool mIsDefault[FolderListView::PROP_MAX];
    FolderListView *mFolderListView;
};


#endif

// kate: space-indent on; indent-width 2; replace-tabs on;

/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef PARTBAR_H
#define PARTBAR_H

#include <qlistbox.h>
#include <klistbox.h>
#include <kicontheme.h>

#include <qevent.h>

//#include "manipulatorpart.h"

namespace KSync {

  /* forward declaration */
    class PartBar;
    class ManipulatorPart;
  
  /**
   * An item for the PartBar
   */
  class PartBarItem : public QListBoxPixmap
    {
    public:
      PartBarItem( PartBar*, ManipulatorPart * );
      ~PartBarItem();

      /**
       * the part to be embedded
       */
      ManipulatorPart* part();

      /**
       * sets the icon for the item.
       * @param icon the icon to set
       * @param group the icongroup
       */
      void setIcon( const QString& icon, KIcon::Group group = KIcon::Panel );


      /**
       * @return the width of this item.
       */
      virtual int width( const QListBox * ) const;
      
      /**
       * @return the height of this item.
       */
      virtual int height( const QListBox * ) const;

      /**
       * return the pixmap.
       */
      virtual const QPixmap * pixmap() const {
	return m_Pixmap;
      }

    protected:
      virtual void paint( QPainter *p);

    private:
      QPixmap* m_Pixmap;
      ManipulatorPart* m_Part;
      PartBar* m_Parents;
    };


  //class KListBox;

  /**
   * PartBar is a widget that displays icons together.
   * The the items of the PartBar emit the activated() signal.
   */
  class PartBar  : public QFrame
    {
      Q_OBJECT
    public:
      PartBar( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
      ~PartBar() {};

      virtual PartBarItem* insertItem( ManipulatorPart *part, int pos = -1 );

      /**
       * Allows to set a custom KListBox
       * Items of the previous box will not be moved to the next box
       */
      virtual void setListBox(  KListBox*);

      /**
       * @returns the KListBox that is used.
       */
      KListBox *listBox() const { return m_listBox; }

      /**
       * removes all items
       */
      virtual void clear();
      
      /**
       * @return a size hint.
       */
      virtual QSize sizeHint() const;

      virtual QSize minimumSizeHint() const;

      /**
       * @returns the current PartBarItem
       */
      PartBarItem *currentItem() const;

      void selectPart( const QString & );

    signals:
      void activated( ManipulatorPart *part );

    protected slots:
      virtual void slotSelected( QListBoxItem * );

    protected:
      virtual void resizeEvent( QResizeEvent * );

    private:
      KListBox *m_listBox;
      PartBarItem *m_activeItem;
    };

}

#endif

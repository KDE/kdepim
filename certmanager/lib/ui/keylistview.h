/*  -*- mode: C++; c-file-style: "gnu" -*-
    keylistview.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO_KEYLISTVIEW_H__
#define __KLEO_KEYLISTVIEW_H__

#include <klistview.h>

#include <gpgmepp/key.h>

namespace Kleo {

  class KeyListView;

  class KeyListViewItem : public KListViewItem {
  public:
    KeyListViewItem( KeyListView * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key );

    void setKey( const GpgME::Key & key );
    const GpgME::Key & key() const { return mKey; }

    enum { RTTI = 86172450 };

    //
    // only boring stuff below:
    //
    /*! \reimp for covariant return */
    KeyListView * listView() const;
    /*! \reimp */
    QString text( int col ) const;
    /*! \reimp */
    const QPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( QListViewItem * other, int col, bool ascending ) const;
    /*! \reimp to allow for key() overload above */
    QString key( int col, bool ascending ) const { return KListViewItem::key( col, ascending ); }
    /*! \reimp */
    int rtti() const { return RTTI; }

  private:
    GpgME::Key mKey;
  };

  class KeyListView : public KListView {
    Q_OBJECT
  public:
    class ColumnStrategy;

    KeyListView( const ColumnStrategy * strategy, QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~KeyListView();

    const ColumnStrategy * columnStrategy() const { return mColumnStrategy; }

  signals:
    void doubleClicked( Kleo::KeyListViewItem*, const QPoint&, int );
    void returnPressed( Kleo::KeyListViewItem* );

  public slots:
    void slotAddKey( const GpgME::Key & key );

  private slots:
    void slotEmitDoubleClicked( QListViewItem*, const QPoint&, int );
    void slotEmitReturnPressed( QListViewItem* );

    //
    // Only boring stuff below:
    //
  public:
    /*! \reimp for covariant return */
    KeyListViewItem * selectedItem() const;

  private:
    const ColumnStrategy * mColumnStrategy;
  };

  class KeyListView::ColumnStrategy {
  public:
    virtual ~ColumnStrategy();
    virtual QString title( int column ) const = 0;
    virtual int width( int column, const QFontMetrics & fm ) const;
    virtual QListView::WidthMode widthMode( int ) const { return QListView::Manual; }
    virtual QString text( const GpgME::Key & key, int column ) const = 0;
    virtual const QPixmap * pixmap( const GpgME::Key &, int ) const { return 0; }
    virtual int compare( const GpgME::Key & key1, const GpgME::Key & key2, const int column ) const;
  };

}

#endif // __KLEO_KEYLISTVIEW_H__

/*
    keylistview.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "kleo/kleo_export.h"

#include <gpgme++/key.h>

#include <QtCore/QByteArray>
#include <QTreeWidget>
#include <QHeaderView>
#include <KIcon>

class QPainter;
class QColorGroup;
class QFont;
class QColor;

namespace Kleo {

  // work around moc parser bug...
#define TEMPLATE_TYPENAME(T) template <typename T>
  TEMPLATE_TYPENAME(T)
  inline T * lvi_cast( QTreeWidgetItem * item ) {
    return item && (item->type() == T::RTTI)
      ? static_cast<T*>( item ) : 0 ;
  }

  TEMPLATE_TYPENAME(T)
  inline const T * lvi_cast( const QTreeWidgetItem * item ) {
    return item && (item->type() == T::RTTI)
      ? static_cast<const T*>( item ) : 0 ;
  }
#undef TEMPLATE_TYPENAME

  class KeyListView;

  class KLEO_EXPORT KeyListViewItem : public QTreeWidgetItem {
  public:
    KeyListViewItem( KeyListView * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key );
    ~KeyListViewItem();

    void setKey( const GpgME::Key & key );
    const GpgME::Key & key() const { return mKey; }

    enum { RTTI = QTreeWidgetItem::UserType + 1 };

    //
    // only boring stuff below:
    //
    virtual QString toolTip( int column ) const;

    /*! \reimp for covariant return */
    KeyListView * listView() const;
    /*! \reimp for covariant return */
    KeyListViewItem * nextSibling() const;
    /*! \reimp */
    bool operator<( const QTreeWidgetItem &other ) const;
    /*! \reimp */
    void takeItem( QTreeWidgetItem* item );

  private:
    GpgME::Key mKey;
  };


  class KLEO_EXPORT KeyListView : public QTreeWidget {
    Q_OBJECT
    friend class KeyListViewItem;
  public:

    class KLEO_EXPORT ColumnStrategy {
    public:
      virtual ~ColumnStrategy();
      virtual QString title( int column ) const = 0;
      virtual int width( int column, const QFontMetrics & fm ) const;
      virtual QHeaderView::ResizeMode resizeMode( int ) const { return QHeaderView::Interactive; }

      virtual QString text( const GpgME::Key & key, int column ) const = 0;
      virtual QString toolTip( const GpgME::Key & key, int column ) const;
      virtual KIcon icon( const GpgME::Key &, int ) const { return KIcon(); }
      virtual int compare( const GpgME::Key & key1, const GpgME::Key & key2, const int column ) const;
    };

    class KLEO_EXPORT DisplayStrategy {
    public:
      virtual ~DisplayStrategy();
      //font
      virtual QFont keyFont( const GpgME::Key &, const QFont & ) const;
      //foreground
      virtual QColor keyForeground( const GpgME::Key & , const QColor & ) const;
      //background
      virtual QColor keyBackground( const GpgME::Key &, const QColor &  ) const;
    };

    explicit KeyListView( const ColumnStrategy * strategy,
                 const DisplayStrategy * display=0,
                 QWidget * parent=0, Qt::WindowFlags f=0 );

    ~KeyListView();

    const ColumnStrategy * columnStrategy() const { return mColumnStrategy; }
    const DisplayStrategy * displayStrategy() const { return mDisplayStrategy; }

    bool hierarchical() const { return mHierarchical; }
    virtual void setHierarchical( bool hier );

    void flushKeys() { slotUpdateTimeout(); }

    bool isMultiSelection() const;

    KeyListViewItem * itemByFingerprint( const QByteArray & ) const;

    using QTreeWidget::selectionChanged; // for below, but moc doesn't like it to be in the signals: section
  Q_SIGNALS:
    void doubleClicked( Kleo::KeyListViewItem*, int );
    void returnPressed( Kleo::KeyListViewItem* );
    void selectionChanged( Kleo::KeyListViewItem* );
    void contextMenu( Kleo::KeyListViewItem*, const QPoint& );

  protected:
    void keyPressEvent(QKeyEvent* event);

  public Q_SLOTS:
    virtual void slotAddKey( const GpgME::Key & key );
    virtual void slotRefreshKey( const GpgME::Key & key );

    //
    // Only boring stuff below:
    //
  private Q_SLOTS:
    void slotEmitDoubleClicked( QTreeWidgetItem*, int );
    void slotEmitReturnPressed( QTreeWidgetItem* );
    void slotEmitSelectionChanged();
    void slotEmitContextMenu( const QPoint& pos );
    void slotUpdateTimeout();

  public:
    /*! \reimp for covariant return */
    KeyListViewItem * selectedItem() const;
    /*! \reimp */
    QList<KeyListViewItem*> selectedItems() const;
    /*! \reimp for covariant return */
    KeyListViewItem * firstChild() const;
    /*! \reimp */
    void clear();
    /*! \reimp */
    void takeItem( QTreeWidgetItem * );

  private:
    void doHierarchicalInsert( const GpgME::Key & );
    void gatherScattered();
    void scatterGathered( KeyListViewItem* );
    void registerItem( KeyListViewItem * );
    void deregisterItem( const KeyListViewItem * );

  private:
    const ColumnStrategy * mColumnStrategy;
    const DisplayStrategy * mDisplayStrategy;
    bool mHierarchical;

    class Private;
    Private *const d;
  };
}

#endif // __KLEO_KEYLISTVIEW_H__

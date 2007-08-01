/*
    keylistview.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

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

#include <k3listview.h>

#include <QtGui/QPixmap>
#include <QtCore/QByteArray>
#include <Qt3Support/Q3PtrList>

class QPainter;
class QColorGroup;
class QFont;
class QColor;
class QEvent;

namespace Kleo {

  // work around moc parser bug...
#define TEMPLATE_TYPENAME(T) template <typename T>
  TEMPLATE_TYPENAME(T)
  inline T * lvi_cast( Q3ListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<T*>( item ) : 0 ;
  }

  TEMPLATE_TYPENAME(T)
  inline const T * lvi_cast( const Q3ListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<const T*>( item ) : 0 ;
  }
#undef TEMPLATE_TYPENAME

  class KeyListView;

  class KLEO_EXPORT KeyListViewItem : public Q3ListViewItem {
  public:
    KeyListViewItem( KeyListView * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key );
    KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key );
    ~KeyListViewItem();

    void setKey( const GpgME::Key & key );
    const GpgME::Key & key() const { return mKey; }

    enum { RTTI_MASK = 0xFFFFFFF0, RTTI = 0x2C1362E0 };

    //
    // only boring stuff below:
    //
    virtual QString toolTip( int column ) const;

    /*! \reimp for covariant return */
    KeyListView * listView() const;
    /*! \reimp for covariant return */
    KeyListViewItem * nextSibling() const;
    /*! \reimp */
    int compare( Q3ListViewItem * other, int col, bool ascending ) const;
    /*! \reimp to allow for key() overload above */
    QString key( int col, bool ascending ) const { return Q3ListViewItem::key( col, ascending ); }
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );
    /*! \reimp */
    void insertItem( Q3ListViewItem * item );
    /*! \reimp */
    void takeItem( Q3ListViewItem * item );

  private:
    GpgME::Key mKey;
  };

  class KLEO_EXPORT SubkeyKeyListViewItem : public KeyListViewItem {
  public:
    SubkeyKeyListViewItem( KeyListView * parent, const GpgME::Subkey & subkey );
    SubkeyKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Subkey & subkey );
    SubkeyKeyListViewItem( KeyListViewItem * parent, const GpgME::Subkey & subkey );
    SubkeyKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Subkey & subkey );

    void setSubkey( const GpgME::Subkey & subkey );
    const GpgME::Subkey & subkey() const { return mSubkey; }

    enum { RTTI = KeyListViewItem::RTTI + 1 };

    //
    // only boring stuff below:
    //
    /*! \reimp */
    QString toolTip( int col ) const;
    /*! \reimp */
    QString text( int col ) const;
    /*! \reimp */
    const QPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( Q3ListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::Subkey mSubkey;
  };

  class KLEO_EXPORT UserIDKeyListViewItem : public KeyListViewItem {
  public:
    UserIDKeyListViewItem( KeyListView * parent, const GpgME::UserID & userid );
    UserIDKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::UserID & userid );
    UserIDKeyListViewItem( KeyListViewItem * parent, const GpgME::UserID & userid );
    UserIDKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::UserID & userid );

    void setUserID( const GpgME::UserID & userid );
    const GpgME::UserID userID() const { return mUserID; }

    enum { RTTI = KeyListViewItem::RTTI + 2 };

    //
    // only boring stuff below:
    //
    /*! \reimp */
    QString toolTip( int col ) const;
    /*! \reimp */
    QString text( int col ) const;
    /*! \reimp */
    const QPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( Q3ListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID mUserID;
  };

  class KLEO_EXPORT SignatureKeyListViewItem : public KeyListViewItem {
  public:
    SignatureKeyListViewItem( KeyListView * parent, const GpgME::UserID::Signature & sig );
    SignatureKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::UserID::Signature & sig );
    SignatureKeyListViewItem( KeyListViewItem * parent, const GpgME::UserID::Signature & sig );
    SignatureKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::UserID::Signature & sig );

    void setSignature( const GpgME::UserID::Signature & sig );
    const GpgME::UserID::Signature & signature() const { return mSignature; }

    enum { RTTI = KeyListViewItem::RTTI + 3 };

    //
    // only boring stuff below:
    //
    /*! \reimp */
    QString toolTip( int col ) const;
    /*! \reimp */
    QString text( int col ) const;
    /*! \reimp */
    const QPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( Q3ListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID::Signature mSignature;
  };


  class KLEO_EXPORT KeyListView : public K3ListView {
    Q_OBJECT
    friend class KeyListViewItem;
  public:

    class KLEO_EXPORT ColumnStrategy {
    public:
      virtual ~ColumnStrategy();
      virtual QString title( int column ) const = 0;
      virtual int width( int column, const QFontMetrics & fm ) const;
      virtual Q3ListView::WidthMode widthMode( int ) const { return Q3ListView::Manual; }

      virtual QString text( const GpgME::Key & key, int column ) const = 0;
      virtual QString toolTip( const GpgME::Key & key, int column ) const;
      virtual const QPixmap * pixmap( const GpgME::Key &, int ) const { return 0; }
      virtual int compare( const GpgME::Key & key1, const GpgME::Key & key2, const int column ) const;

      virtual QString subkeyText( const GpgME::Subkey &, int ) const { return QString(); }
      virtual QString subkeyToolTip( const GpgME::Subkey & subkey, int column ) const;
      virtual const QPixmap * subkeyPixmap( const GpgME::Subkey &, int ) const { return 0; }
      virtual int subkeyCompare( const GpgME::Subkey & subkey1, const GpgME::Subkey & subkey2, const int column ) const;

      virtual QString userIDText( const GpgME::UserID &, int ) const { return QString(); }
      virtual QString userIDToolTip( const GpgME::UserID & userID, int column ) const;
      virtual const QPixmap * userIDPixmap( const GpgME::UserID &, int ) const { return 0; }
      virtual int userIDCompare( const GpgME::UserID & userID1, const GpgME::UserID & userID2, const int column ) const;

      virtual QString signatureText( const GpgME::UserID::Signature &, int ) const { return QString(); }
      virtual QString signatureToolTip( const GpgME::UserID::Signature & sig, int column ) const;
      virtual const QPixmap * signaturePixmap( const GpgME::UserID::Signature &, int ) const { return 0; }
      virtual int signatureCompare( const GpgME::UserID::Signature & sig1, const GpgME::UserID::Signature & sig2, const int column ) const;      
    };

    class KLEO_EXPORT DisplayStrategy {
    public:
      virtual ~DisplayStrategy();
      //font
      virtual QFont keyFont( const GpgME::Key &, const QFont & ) const;
      virtual QFont subkeyFont( const GpgME::Subkey &, const QFont & ) const;
      virtual QFont useridFont( const GpgME::UserID &, const QFont &  ) const;
      virtual QFont signatureFont( const GpgME::UserID::Signature & , const QFont & ) const;
      //foreground
      virtual QColor keyForeground( const GpgME::Key & , const QColor & ) const;
      virtual QColor subkeyForeground( const GpgME::Subkey &, const QColor &  ) const;
      virtual QColor useridForeground( const GpgME::UserID &, const QColor &  ) const;
      virtual QColor signatureForeground( const GpgME::UserID::Signature &, const QColor &  ) const;
      //background
      virtual QColor keyBackground( const GpgME::Key &, const QColor &  ) const;
      virtual QColor subkeyBackground( const GpgME::Subkey &, const QColor &  ) const;
      virtual QColor useridBackground( const GpgME::UserID &, const QColor & ) const;
      virtual QColor signatureBackground( const GpgME::UserID::Signature &, const QColor &  ) const;
    };

    KeyListView( const ColumnStrategy * strategy,
		 const DisplayStrategy * display=0,
		 QWidget * parent=0, Qt::WFlags f=0 );

    ~KeyListView();

    const ColumnStrategy * columnStrategy() const { return mColumnStrategy; }
    const DisplayStrategy * displayStrategy() const { return mDisplayStrategy; }

    bool hierarchical() const { return mHierarchical; }
    virtual void setHierarchical( bool hier );

    void flushKeys() { slotUpdateTimeout(); }

    bool hasSelection() const;

    KeyListViewItem * itemByFingerprint( const QByteArray & ) const;

  Q_SIGNALS:
    void doubleClicked( Kleo::KeyListViewItem*, const QPoint&, int );
    void returnPressed( Kleo::KeyListViewItem* );
    void selectionChanged( Kleo::KeyListViewItem* );
    void contextMenu( Kleo::KeyListViewItem*, const QPoint& );

  public Q_SLOTS:
    virtual void slotAddKey( const GpgME::Key & key );
    virtual void slotRefreshKey( const GpgME::Key & key );

    //
    // Only boring stuff below:
    //
  private Q_SLOTS:
    void slotEmitDoubleClicked( Q3ListViewItem*, const QPoint&, int );
    void slotEmitReturnPressed( Q3ListViewItem* );
    void slotEmitSelectionChanged( Q3ListViewItem* );
    void slotEmitContextMenu( K3ListView*, Q3ListViewItem*, const QPoint& );
    void slotUpdateTimeout();

  public:
    /*! \reimp for covariant return */
    KeyListViewItem * selectedItem() const;
    /*! \reimp */
    Q3PtrList<KeyListViewItem> selectedItems() const;
    /*! \reimp for covariant return */
    KeyListViewItem * firstChild() const;
    /*! \reimp */
    void clear();
    /*! \reimp */
    void insertItem( Q3ListViewItem * );
    /*! \reimp */
    void takeItem( Q3ListViewItem * );

  protected:
    virtual bool event(QEvent *e );
  private:
    bool showToolTip( const QPoint& p );
    void doHierarchicalInsert( const GpgME::Key & );
    void gatherScattered();
    void scatterGathered( Q3ListViewItem * );
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

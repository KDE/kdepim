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

class QPainter;
class QColorGroup;
class QFont;
class QColor;

namespace Kleo {

  // work around moc parser bug...
#define TEMPLATE_TYPENAME(T) template <typename T>
  TEMPLATE_TYPENAME(T)
  inline T * lvi_cast( QListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<T*>( item ) : 0 ;
  }

  TEMPLATE_TYPENAME(T)
  inline const T * lvi_cast( const QListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<const T*>( item ) : 0 ;
  }
#undef TEMPLATE_TYPENAME

  class KeyListView;

  class KeyListViewItem : public QListViewItem {
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
    int compare( QListViewItem * other, int col, bool ascending ) const;
    /*! \reimp to allow for key() overload above */
    QString key( int col, bool ascending ) const { return QListViewItem::key( col, ascending ); }
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );
    /*! \reimp */
    void insertItem( QListViewItem * item );
    /*! \reimp */
    void takeItem( QListViewItem * item );

  private:
    GpgME::Key mKey;
  };

  class SubkeyKeyListViewItem : public KeyListViewItem {
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
    int compare( QListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::Subkey mSubkey;
  };

  class UserIDKeyListViewItem : public KeyListViewItem {
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
    int compare( QListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID mUserID;
  };

  class SignatureKeyListViewItem : public KeyListViewItem {
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
    int compare( QListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID::Signature mSignature;
  };


  class KeyListView : public KListView {
    Q_OBJECT
    friend class KeyListViewItem;
  public:
    class ColumnStrategy;
    class DisplayStrategy;


    KeyListView( const ColumnStrategy * strategy,
		 const DisplayStrategy * display=0,
		 QWidget * parent=0, const char * name=0, WFlags f=0 );

    ~KeyListView();

    const ColumnStrategy * columnStrategy() const { return mColumnStrategy; }
    const DisplayStrategy * displayStrategy() const { return mDisplayStrategy; }

    bool hierarchical() const { return mHierarchical; }
    virtual void setHierarchical( bool hier );

    void flushKeys() { slotUpdateTimeout(); }

    bool hasSelection() const;

    KeyListViewItem * itemByFingerprint( const QCString & ) const;

  signals:
    void doubleClicked( Kleo::KeyListViewItem*, const QPoint&, int );
    void returnPressed( Kleo::KeyListViewItem* );
    void selectionChanged( Kleo::KeyListViewItem* );
    void contextMenu( Kleo::KeyListViewItem*, const QPoint& );

  public slots:
    virtual void slotAddKey( const GpgME::Key & key );
    virtual void slotRefreshKey( const GpgME::Key & key );

    //
    // Only boring stuff below:
    //
  private slots:
    void slotEmitDoubleClicked( QListViewItem*, const QPoint&, int );
    void slotEmitReturnPressed( QListViewItem* );
    void slotEmitSelectionChanged( QListViewItem* );
    void slotEmitContextMenu( KListView*, QListViewItem*, const QPoint& );
    void slotUpdateTimeout();

  public:
    /*! \reimp for covariant return */
    KeyListViewItem * selectedItem() const;
    /*! \reimp */
    QPtrList<KeyListViewItem> selectedItems() const;
    /*! \reimp for covariant return */
    KeyListViewItem * firstChild() const;
    /*! \reimp */
    void clear();
    /*! \reimp */
    void insertItem( QListViewItem * );
    /*! \reimp */
    void takeItem( QListViewItem * );

  private:
    void doHierarchicalInsert( const GpgME::Key & );
    void gatherScattered();
    void scatterGathered( QListViewItem * );
    void registerItem( KeyListViewItem * );
    void deregisterItem( const KeyListViewItem * );

  private:
    const ColumnStrategy * mColumnStrategy;
    const DisplayStrategy * mDisplayStrategy;
    bool mHierarchical;

    class Private;
    Private * d;
  };

  class KeyListView::ColumnStrategy {
  public:
    virtual ~ColumnStrategy();
    virtual QString title( int column ) const = 0;
    virtual int width( int column, const QFontMetrics & fm ) const;
    virtual QListView::WidthMode widthMode( int ) const { return QListView::Manual; }

    virtual QString text( const GpgME::Key & key, int column ) const = 0;
    virtual QString toolTip( const GpgME::Key & key, int column ) const;
    virtual const QPixmap * pixmap( const GpgME::Key &, int ) const { return 0; }
    virtual int compare( const GpgME::Key & key1, const GpgME::Key & key2, const int column ) const;

    virtual QString subkeyText( const GpgME::Subkey &, int ) const { return QString::null; }
    virtual QString subkeyToolTip( const GpgME::Subkey & subkey, int column ) const;
    virtual const QPixmap * subkeyPixmap( const GpgME::Subkey &, int ) const { return 0; }
    virtual int subkeyCompare( const GpgME::Subkey & subkey1, const GpgME::Subkey & subkey2, const int column ) const;

    virtual QString userIDText( const GpgME::UserID &, int ) const { return QString::null; }
    virtual QString userIDToolTip( const GpgME::UserID & userID, int column ) const;
    virtual const QPixmap * userIDPixmap( const GpgME::UserID &, int ) const { return 0; }
    virtual int userIDCompare( const GpgME::UserID & userID1, const GpgME::UserID & userID2, const int column ) const;

    virtual QString signatureText( const GpgME::UserID::Signature &, int ) const { return QString::null; }
    virtual QString signatureToolTip( const GpgME::UserID::Signature & sig, int column ) const;
    virtual const QPixmap * signaturePixmap( const GpgME::UserID::Signature &, int ) const { return 0; }
    virtual int signatureCompare( const GpgME::UserID::Signature & sig1, const GpgME::UserID::Signature & sig2, const int column ) const;
  };

  class KeyListView::DisplayStrategy {
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

}

#endif // __KLEO_KEYLISTVIEW_H__

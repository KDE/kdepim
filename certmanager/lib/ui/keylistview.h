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

#include <klistview.h>

#include <gpgmepp/key.h>
#include <kdepimmacros.h>

class QPainter;
class QColorGroup;
class QFont;
class QColor;

namespace Kleo {

  // work around moc parser bug...
#define TEMPLATE_TYPENAME(T) template <typename T>
  TEMPLATE_TYPENAME(T)
  inline T * lvi_cast( TQListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<T*>( item ) : 0 ;
  }

  TEMPLATE_TYPENAME(T)
  inline const T * lvi_cast( const TQListViewItem * item ) {
    return item && (item->rtti() & T::RTTI_MASK) == T::RTTI
      ? static_cast<const T*>( item ) : 0 ;
  }
#undef TEMPLATE_TYPENAME

  class KeyListView;

  class KDE_EXPORT KeyListViewItem : public TQListViewItem {
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
    virtual TQString toolTip( int column ) const;

    /*! \reimp for covariant return */
    KeyListView * listView() const;
    /*! \reimp for covariant return */
    KeyListViewItem * nextSibling() const;
    /*! \reimp */
    int compare( TQListViewItem * other, int col, bool ascending ) const;
    /*! \reimp to allow for key() overload above */
    TQString key( int col, bool ascending ) const { return TQListViewItem::key( col, ascending ); }
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment );
    /*! \reimp */
    void insertItem( TQListViewItem * item );
    /*! \reimp */
    void takeItem( TQListViewItem * item );

  private:
    GpgME::Key mKey;
  };

  class KDE_EXPORT SubkeyKeyListViewItem : public KeyListViewItem {
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
    TQString toolTip( int col ) const;
    /*! \reimp */
    TQString text( int col ) const;
    /*! \reimp */
    const TQPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( TQListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::Subkey mSubkey;
  };

  class KDE_EXPORT UserIDKeyListViewItem : public KeyListViewItem {
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
    TQString toolTip( int col ) const;
    /*! \reimp */
    TQString text( int col ) const;
    /*! \reimp */
    const TQPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( TQListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID mUserID;
  };

  class KDE_EXPORT SignatureKeyListViewItem : public KeyListViewItem {
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
    TQString toolTip( int col ) const;
    /*! \reimp */
    TQString text( int col ) const;
    /*! \reimp */
    const TQPixmap * pixmap( int col ) const;
    /*! \reimp */
    int compare( TQListViewItem * other, int col, bool ascending ) const;
    /*! \reimp */
    int rtti() const { return RTTI; }
    /*! \reimp */
    void paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment );

  private:
    GpgME::UserID::Signature mSignature;
  };


  class KDE_EXPORT KeyListView : public KListView {
    Q_OBJECT
    friend class KeyListViewItem;
  public:

    class KDE_EXPORT ColumnStrategy {
    public:
      virtual ~ColumnStrategy();
      virtual TQString title( int column ) const = 0;
      virtual int width( int column, const TQFontMetrics & fm ) const;
      virtual TQListView::WidthMode widthMode( int ) const { return TQListView::Manual; }

      virtual TQString text( const GpgME::Key & key, int column ) const = 0;
      virtual TQString toolTip( const GpgME::Key & key, int column ) const;
      virtual const TQPixmap * pixmap( const GpgME::Key &, int ) const { return 0; }
      virtual int compare( const GpgME::Key & key1, const GpgME::Key & key2, const int column ) const;

      virtual TQString subkeyText( const GpgME::Subkey &, int ) const { return TQString::null; }
      virtual TQString subkeyToolTip( const GpgME::Subkey & subkey, int column ) const;
      virtual const TQPixmap * subkeyPixmap( const GpgME::Subkey &, int ) const { return 0; }
      virtual int subkeyCompare( const GpgME::Subkey & subkey1, const GpgME::Subkey & subkey2, const int column ) const;

      virtual TQString userIDText( const GpgME::UserID &, int ) const { return TQString::null; }
      virtual TQString userIDToolTip( const GpgME::UserID & userID, int column ) const;
      virtual const TQPixmap * userIDPixmap( const GpgME::UserID &, int ) const { return 0; }
      virtual int userIDCompare( const GpgME::UserID & userID1, const GpgME::UserID & userID2, const int column ) const;

      virtual TQString signatureText( const GpgME::UserID::Signature &, int ) const { return TQString::null; }
      virtual TQString signatureToolTip( const GpgME::UserID::Signature & sig, int column ) const;
      virtual const TQPixmap * signaturePixmap( const GpgME::UserID::Signature &, int ) const { return 0; }
      virtual int signatureCompare( const GpgME::UserID::Signature & sig1, const GpgME::UserID::Signature & sig2, const int column ) const;
    };

    class KDE_EXPORT DisplayStrategy {
    public:
      virtual ~DisplayStrategy();
      //font
      virtual TQFont keyFont( const GpgME::Key &, const TQFont & ) const;
      virtual TQFont subkeyFont( const GpgME::Subkey &, const TQFont & ) const;
      virtual TQFont useridFont( const GpgME::UserID &, const TQFont &  ) const;
      virtual TQFont signatureFont( const GpgME::UserID::Signature & , const TQFont & ) const;
      //foreground
      virtual TQColor keyForeground( const GpgME::Key & , const TQColor & ) const;
      virtual TQColor subkeyForeground( const GpgME::Subkey &, const TQColor &  ) const;
      virtual TQColor useridForeground( const GpgME::UserID &, const TQColor &  ) const;
      virtual TQColor signatureForeground( const GpgME::UserID::Signature &, const TQColor &  ) const;
      //background
      virtual TQColor keyBackground( const GpgME::Key &, const TQColor &  ) const;
      virtual TQColor subkeyBackground( const GpgME::Subkey &, const TQColor &  ) const;
      virtual TQColor useridBackground( const GpgME::UserID &, const TQColor & ) const;
      virtual TQColor signatureBackground( const GpgME::UserID::Signature &, const TQColor &  ) const;
    };

    KeyListView( const ColumnStrategy * strategy,
		 const DisplayStrategy * display=0,
		 TQWidget * parent=0, const char * name=0, WFlags f=0 );

    ~KeyListView();

    const ColumnStrategy * columnStrategy() const { return mColumnStrategy; }
    const DisplayStrategy * displayStrategy() const { return mDisplayStrategy; }

    bool hierarchical() const { return mHierarchical; }
    virtual void setHierarchical( bool hier );

    void flushKeys() { slotUpdateTimeout(); }

    bool hasSelection() const;

    KeyListViewItem * itemByFingerprint( const TQCString & ) const;

  signals:
    void doubleClicked( Kleo::KeyListViewItem*, const TQPoint&, int );
    void returnPressed( Kleo::KeyListViewItem* );
    void selectionChanged( Kleo::KeyListViewItem* );
    void contextMenu( Kleo::KeyListViewItem*, const TQPoint& );

  public slots:
    virtual void slotAddKey( const GpgME::Key & key );
    virtual void slotRefreshKey( const GpgME::Key & key );

    //
    // Only boring stuff below:
    //
  private slots:
    void slotEmitDoubleClicked( TQListViewItem*, const TQPoint&, int );
    void slotEmitReturnPressed( TQListViewItem* );
    void slotEmitSelectionChanged( TQListViewItem* );
    void slotEmitContextMenu( KListView*, TQListViewItem*, const TQPoint& );
    void slotUpdateTimeout();

  public:
    /*! \reimp for covariant return */
    KeyListViewItem * selectedItem() const;
    /*! \reimp */
    TQPtrList<KeyListViewItem> selectedItems() const;
    /*! \reimp for covariant return */
    KeyListViewItem * firstChild() const;
    /*! \reimp */
    void clear();
    /*! \reimp */
    void insertItem( TQListViewItem * );
    /*! \reimp */
    void takeItem( TQListViewItem * );

  private:
    void doHierarchicalInsert( const GpgME::Key & );
    void gatherScattered();
    void scatterGathered( TQListViewItem * );
    void registerItem( KeyListViewItem * );
    void deregisterItem( const KeyListViewItem * );

  private:
    const ColumnStrategy * mColumnStrategy;
    const DisplayStrategy * mDisplayStrategy;
    bool mHierarchical;

    class Private;
    Private * d;
  };
}

#endif // __KLEO_KEYLISTVIEW_H__

/* -*- mode: C++; c-file-style: "gnu" -*-
 * KMAtmListViewItem Header File
 * Author: Markus Wuebben <markus.wuebben@kde.org>
 */
#ifndef __KMAIL_KMATMLISTVIEW_H__
#define __KMAIL_KMATMLISTVIEW_H__

#include <tqlistview.h>
#include <tqcstring.h>

class KMComposeWin;
class MessageComposer;
class TQCheckBox;

class KMAtmListViewItem : public TQObject, public QListViewItem
{
  Q_OBJECT

public:
  KMAtmListViewItem( TQListView *parent );
  virtual ~KMAtmListViewItem();

  //A custom compare function is needed because the size column is
  //human-readable and therefore doesn't sort correctly.
  virtual int compare( TQListViewItem *i, int col, bool ascending ) const;

  virtual void paintCell ( TQPainter * p, const TQColorGroup & cg, int column, int width, int align );

  void setUncompressedMimeType( const TQCString & type, const TQCString & subtype ) {
    mType = type; mSubtype = subtype;
  }
  void setAttachmentSize( int numBytes ) {
    mAttachmentSize = numBytes;
  }
  void uncompressedMimeType( TQCString & type, TQCString & subtype ) const {
    type = mType; subtype = mSubtype;
  }
  void setUncompressedCodec( const TQCString &codec ) { mCodec = codec; }
  TQCString uncompressedCodec() const { return mCodec; }

  void enableCryptoCBs( bool on );
  void setEncrypt( bool on );
  bool isEncrypt();
  void setSign( bool on );
  bool isSign();
  void setCompress( bool on );
  bool isCompress();

signals:
  void compress( int );
  void uncompress( int );

private slots:
  void slotCompress();
  void slotHeaderChange( int, int, int );
  void slotHeaderClick( int );

protected:

  void updateCheckBox( int headerSection, TQCheckBox *cb );
  void updateAllCheckBoxes();

private:
  TQCheckBox *mCBEncrypt;
  TQCheckBox *mCBSign;
  TQCheckBox *mCBCompress;
  TQCString mType, mSubtype, mCodec;
  int mAttachmentSize;
};

#endif // __KMAIL_KMATMLISTVIEW_H__

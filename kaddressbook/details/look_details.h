/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef LOOK_DETAILS_H
#define LOOK_DETAILS_H

#include <kabc/addressbook.h>
#include <kaction.h>
#include <klocale.h>

#include <qmap.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qrect.h>

#include "look_basic.h"

class KABEntryPainter;
class QComboBox;

/**
  This class implements the detailed view.
  Currently, there is no possibility to change the entry in this
  view.
 */

class KABDetailedView : public KABBasicLook
{
  Q_OBJECT

  public:
    /**
      Enum to select how the background is drawn.
     */
    enum BackgroundStyle
    {
      None,
      Tiled,
      Bordered
    };

    /**
      The constructor.
     */
    KABDetailedView( QWidget *parent = 0, const char* name = 0 );

    /**
      The virtual destructor.
     */
    virtual ~KABDetailedView();

    /**
      Set the addressee.
     */
    void setAddressee( const KABC::Addressee& );

    /**
      Overloaded from KABBasicLook.
     */
    void setReadOnly( bool );

    /**
      Overloaded from KABBasicLook.
     */
    void restoreSettings( KConfig* );

  public slots:
    void slotBorderedBGSelected( int index );
    void slotTiledBGSelected( int index );

  protected:
    void paintEvent( QPaintEvent* );
    void mousePressEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );

    /**
      A method to retrieve a background image according to the path
      stored in the entry. It is either loaded
      from backgrounds, that acts as a cache, or from the file
      and added to @see backgrounds.
    */
    bool getBackground( QString path, QPixmap& image );

  private:
    QPtrList<QRect> mURLRects;
    QPtrList<QRect> mEmailRects;
    QPtrList<QRect> mPhoneRects;
    KABEntryPainter *mPainter;

    QMap<QString, QPixmap> mBackgroundMap;
    QPixmap mCurrentBackground;

    BackgroundStyle mBackgroundStyle;

    bool mUseDefaultBGImage;
    bool mUseHeadLineBGColor;

    QColor mDefaultBGColor;
    QColor mHeadLineBGColor;
    QColor mHeadLineTextColor;

    QPixmap mDefaultBGImage;

    KToggleAction *mActionShowAddresses;
    KToggleAction *mActionShowEmails;
    KToggleAction *mActionShowPhones;
    KToggleAction *mActionShowURLs;

    const int mGrid;
    QStringList mBorders;
    QStringList mTiles;

    QPopupMenu *mMenuBorderedBG;
    QPopupMenu *mMenuTiledBG;

    static const QString mBorderedBGDir;
    static const QString mTiledBGDir;
};

class KABDetailedViewFactory : public KABLookFactory
{
  public:
    KABDetailedViewFactory( QWidget *parent = 0, const char *name = 0 )
      : KABLookFactory( parent, name ) {}

    KABBasicLook *create()
    {
      return new KABDetailedView( mParent, mName );
    }

    QString description()
    {
      return i18n( "Detailed Style: Display all details, no modifications." );
    }
};

#endif

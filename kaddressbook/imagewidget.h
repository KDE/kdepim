/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <qlabel.h>

#include <kabc/picture.h>
#include <kdialogbase.h>

#include "contacteditorwidget.h"

class KURLRequester;
class QCheckBox;

#include <librss/global.h>

namespace RSS {
class Loader;
class Document;
}

using namespace RSS;

/**
  Small helper class
 */
class ImageLabel : public QLabel
{
  Q_OBJECT

  public:
    ImageLabel( const QString &title, QWidget *parent );

    void setReadOnly( bool readOnly );

  signals:
    void changed();
    void urlDropped( const KURL& );

  protected:
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );

  private:
    void startDrag();

    bool mReadOnly;
    QPoint mDragStartPos;
};

class ImageBaseWidget : public QWidget
{
  Q_OBJECT

  public:
    ImageBaseWidget( const QString &title, QWidget *parent, const char *name = 0 );
    ~ImageBaseWidget();

    /**
      Show/hide button for getting image from blog feed.
    */
    void showBlogButton( bool show );

    /**
      Set URL of blog feed for getting the image.
    */
    void setBlogFeed( const QString & );

    /**
      Sets the photo object.
     */
    void setImage( const KABC::Picture &photo );

    /**
      Returns a photo object.
     */
    KABC::Picture image() const;

    void setReadOnly( bool readOnly );

  signals:
    void changed();

  public slots:
    void urlDropped( const KURL& );

  private slots:
    void loadImage();
    void updateGUI();
    void clear();
    void imageChanged();
    void getPictureFromBlog();
    void slotLoadingComplete( Loader *loader, Document doc, Status status );

  private:
    QPixmap loadPixmap( const KURL &url );

    ImageLabel *mImageLabel;
    KURLRequester *mImageUrl;

    QCheckBox *mUseImageUrl;
    QPushButton *mClearButton;

    QPushButton *mBlogButton;
    QString mBlogFeed;

    bool mReadOnly;

    RSS::Loader *mRssLoader;
};

class ImageWidget : public KAB::ContactEditorWidget
{
  public:
    ImageWidget( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

    int logicalWidth() const { return 2; }

  private:
    ImageBaseWidget *mPhotoWidget;
    ImageBaseWidget *mLogoWidget;
};

class ImageWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
    {
      return new ImageWidget( ab, parent, name );
    }

    QString pageIdentifier() const { return "misc"; }
};

#endif

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QLabel>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QDropEvent>

#include <kabc/picture.h>
#include <kdialog.h>

#include "contacteditorwidget.h"

class KUrlRequester;
class QCheckBox;

#include <syndication/global.h>
#include <syndication/sharedptr.h>

namespace Syndication {
class Loader;
class Feed;
}

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
    void urlDropped( const KUrl& );

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
    ImageBaseWidget( const QString &title, QWidget *parent );
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
    void urlDropped( const KUrl& );

  private slots:
    void loadImage();
    void updateGUI();
    void clear();
    void imageChanged();
    void getPictureFromBlog();
    void slotLoadingComplete( Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode error );

  private:
    QPixmap loadPixmap( const KUrl &url );

    ImageLabel *mImageLabel;
    KUrlRequester *mImageUrl;

    QCheckBox *mUseImageUrl;
    QPushButton *mClearButton;

    QPushButton *mBlogButton;
    QString mBlogFeed;

    bool mReadOnly;

    Syndication::Loader *mRssLoader;
};

class ImageWidget : public KAB::ContactEditorWidget
{
  public:
    ImageWidget( KABC::AddressBook *ab, QWidget *parent );

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
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent )
    {
      return new ImageWidget( ab, parent );
    }

    QString pageIdentifier() const { return "misc"; }
};

#endif

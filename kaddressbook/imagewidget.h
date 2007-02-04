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

#include <qlabel.h>
#include <qpushbutton.h>

#include <kabc/picture.h>
#include <kdialogbase.h>

#include "contacteditorwidget.h"

/**
  Small helper class
 */
class ImageLoader : public QObject
{
  Q_OBJECT

  public:
    ImageLoader();

    KABC::Picture loadPicture( const KURL &url, bool *ok );

  private:
    KABC::Picture mPicture;
};

/**
  Small helper class
 */
class ImageButton : public QPushButton
{
  Q_OBJECT

  public:
    ImageButton( const QString &title, QWidget *parent );

    void setReadOnly( bool readOnly );

    void setPicture( const KABC::Picture &picture );
    KABC::Picture picture() const;

    void setImageLoader( ImageLoader *loader );

  signals:
    void changed();
    void urlDropped( const KURL& );

  protected:
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void contextMenuEvent( QContextMenuEvent *event );

  private slots:
    void load();
    void clear();

  private:
    void startDrag();
    void updateGUI();

    bool mReadOnly;
    QPoint mDragStartPos;
    KABC::Picture mPicture;

    ImageLoader *mImageLoader;
};


class ImageBaseWidget : public QWidget
{
  Q_OBJECT

  public:
    ImageBaseWidget( const QString &title, QWidget *parent, const char *name = 0 );
    ~ImageBaseWidget();

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

  private:
    ImageButton *mImageButton;
    ImageLoader *mImageLoader;

    bool mReadOnly;
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

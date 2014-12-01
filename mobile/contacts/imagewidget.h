/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QtCore/QPoint>
#include <QPushButton>

namespace KContacts
{
class Addressee;
}

class ImageLoader;

class ImageWidget : public QPushButton
{
  Q_OBJECT

  public:
    enum Type {
      Photo,
      Logo
    };

    explicit ImageWidget( QWidget *parent = 0 );
    explicit ImageWidget( Type type, QWidget *parent = 0 );
    ~ImageWidget();

    void setType( Type type );

    void loadContact( const KContacts::Addressee &contact );
    void storeContact( KContacts::Addressee &contact ) const;

    void setReadOnly( bool readOnly );

  protected:
#ifndef QT_NO_DRAGANDDROP
    // image drop handling
    virtual void dragEnterEvent( QDragEnterEvent* ) Q_DECL_OVERRIDE;
    virtual void dropEvent( QDropEvent* ) Q_DECL_OVERRIDE;
#endif

    // image drag handling
    virtual void mousePressEvent( QMouseEvent* ) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent( QMouseEvent* ) Q_DECL_OVERRIDE;
#ifndef QT_NO_CONTEXTMENU
    // context menu handling
    virtual void contextMenuEvent( QContextMenuEvent* ) Q_DECL_OVERRIDE;
#endif

  private Q_SLOTS:
    void updateView();

    void changeImage();
    void saveImage();
    void deleteImage();

  private:
    ImageLoader *imageLoader();

    Type mType;
    QImage mImage;
    bool mHasImage;
    bool mReadOnly;

    QPoint mDragStartPos;
    ImageLoader *mImageLoader;
};

#endif

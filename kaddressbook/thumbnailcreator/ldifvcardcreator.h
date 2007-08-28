/*
    This file is part of KAddressBook.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KADDRESSBOOK_LDIFVCARDCREATOR_H
#define KADDRESSBOOK_LDIFVCARDCREATOR_H

#include <QPixmap>
#include <kio/thumbcreator.h>

class KPixmapSplitter;

class VCard_LDIFCreator : public ThumbCreator
{
  public:
    VCard_LDIFCreator();
    virtual ~VCard_LDIFCreator();
    virtual bool create(const QString &path, int width, int height, QImage &img);
    virtual Flags flags() const;

  private:
    KPixmapSplitter *mSplitter;
    QPixmap mPixmap;

    QString name;
    QString text;
    bool readContents( const QString &path );
    int xborder, yborder;
    QSize pixmapSize;
    bool createImageSmall();
    bool createImageBig();
};

#endif // KADDRESSBOOK_LDIFVCARDCREATOR_H

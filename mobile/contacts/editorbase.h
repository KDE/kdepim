/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

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

#ifndef EDITORBASE_H
#define EDITORBASE_H

#include <QWidget>

namespace KContacts
{
  class Addressee;
}

namespace Akonadi
{
  class ContactMetaData;
}

class EditorBase : public QWidget
{
  Q_OBJECT

  public:
    explicit EditorBase( QWidget *parent = 0 );

    ~EditorBase();

    virtual void loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData &metaData ) = 0;

    virtual void saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData &metaData ) const = 0;
};

#endif

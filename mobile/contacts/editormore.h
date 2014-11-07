/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef EDITORMORE_H
#define EDITORMORE_H

#include "editorbase.h"

class EditorMore : public EditorBase
{
  Q_OBJECT

  public:
    explicit EditorMore( QWidget *parent = 0 );

    ~EditorMore();

    void loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData &metaData );
    void saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData &metaData ) const;

  public Q_SLOTS:
    void updateOrganization( const QString &organization );
    void updateName( const KContacts::Addressee &contact );

  Q_SIGNALS:
    void nameChanged( const KContacts::Addressee &contact );

  private:
    void loadCustomFields( const KContacts::Addressee &contact, const Akonadi::ContactMetaData &metaData );
    void saveCustomFields( KContacts::Addressee &contact, Akonadi::ContactMetaData &metaData ) const;

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void playPronunciation() )
    Q_PRIVATE_SLOT( d, void addCustomField() )
    Q_PRIVATE_SLOT( d, void configureCategories() )
};

#endif

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

#ifndef EDITORGENERAL_H
#define EDITORGENERAL_H

#include "editorbase.h"

namespace Akonadi
{
  class Collection;
}

class EditorGeneral : public EditorBase
{
  Q_OBJECT

  public:
    explicit EditorGeneral( QWidget *parent = 0 );

    ~EditorGeneral();

    void setDefaultCollection( const Akonadi::Collection &collection );

    void loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData &metaData );

    void saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData &metaData ) const;

    Akonadi::Collection selectedCollection() const;

  public Q_SLOTS:
    void updateName( const KContacts::Addressee& );

  Q_SIGNALS:
    void collectionChanged( const Akonadi::Collection &collection );
    void nameChanged( const KContacts::Addressee& );

    void saveClicked();
    void cancelClicked();

    void requestLaunchAccountWizard();

  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void nameTextChanged( const QString& ) )
    Q_PRIVATE_SLOT( d, void addEmailClicked() )
    Q_PRIVATE_SLOT( d, void addPhoneClicked() )
    Q_PRIVATE_SLOT( d, void clearEmailClicked() )
    Q_PRIVATE_SLOT( d, void clearPhoneClicked() )
    Q_PRIVATE_SLOT( d, void availableCollectionsChanged() )
    Q_PRIVATE_SLOT( d, void disableSaveButton() )
};

#endif
